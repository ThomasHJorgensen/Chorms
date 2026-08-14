import numpy as np
from EconModel import EconModelClass
from numpy.polynomial.hermite import hermgauss

class ChormsClass(EconModelClass):

    def settings(self):
        """ fundamental settings """

        self.cpp_filename = 'cppfuncs/model_cpp.cpp'


    def setup(self):
        """ set baseline parameters """

        # unpack
        par = self.par

        par.total_time = 16.0 * 7 * 52 / 1000 # total time in a year
        
        # preferences
        par.crra = 1.5 # risk aversion for consumption
        par.util_public = 1.5 # utility weight on public good
        
        par.util_leisure_w = 4.0 # utility weight on leisure
        par.util_leisure_m = 4.5 # utility weight on leisure
        par.leisure_power_w = 1.9 # curvature of leisure utility
        par.leisure_power_m = 2.1  

        # home production
        par.home_effective_w = 1.0 # effective home production input depends on flexibility at work
        par.home_effective_m = 1.0

        par.home_weight = 1.0 # absolute advantage of women in home production (1.0 is equal)
        par.home_power = 0.4 # complementarity between men and women's input into home production

        par.public_weight = 0.6 # weight on market purchased goods in public good production
        par.public_power = 0.38 # complementarity between home and market goods in public good production

        # norms
        par.norm_pos_w = 0.0
        par.norm_pos_m = 0.0

        par.norm_neg_w = 0.0
        par.norm_neg_m = 0.0

        par.norm_close_w = 0.0
        par.norm_close_m = 0.0

        par.norm_attract_w = 0.0
        par.norm_attract_m = 0.0

        # institutional setting
        par.unemployment = 0.05 #12_000.0*12/1_000.0 
        par.tax = 0.3

        # wage distributions
        par.wage_logmean_w = 1.0
        par.wage_logmean_m = 1.0
        par.num_nodes = 5

        par.wage_chol1 = -0.7 # Cholesky decomposition of the covariance matrix of the log wages
        par.wage_chol2 = 0.25 # Cholesky decomposition of the covariance matrix of the log wages
        par.wage_chol3 = 0.0 #-0.3 # Cholesky decomposition of the covariance matrix of the log wages

        # the covaraince matrix is then determined from the Cholesky decomposition (so these parameters are overwritten)
        par.wage_sigma_w = 0.0 # standard deviation of the log wage distribution for women
        par.wage_sigma_m = 0.0 # standard deviation of the log wage distribution for men
        par.wage_cov = 0.0 # covariance of the log wage distribution

        # measurement error variances (std dev) of each outcome
        par.meas_sigma_labor_w = 0.1
        par.meas_sigma_labor_m = 0.1
        par.meas_sigma_home_w = 0.1
        par.meas_sigma_home_m = 0.1
        par.meas_sigma_market = 0.1
        par.meas_sigma_home_share_exp_w = 0.1

        par.meas_sigma_vec = np.nan + np.ones(6)

        # numerical specs 
        par.cpp_threads = 20
        par.reuse_init = True # whether to re-use the initial values from the previous iteration as starting values for the next iteration
        par.do_gridsearch = False
        par.num_grid = 35

        # data and estimation
        par.num_N = 5000
        par.seed = 9210

        par.do_meas_sigma_profile = True

        self.params_wage = ['wage_logmean_w','wage_logmean_m','wage_chol1','wage_chol2','wage_chol3']
        self.params_meas = ['meas_sigma_labor_w','meas_sigma_labor_m','meas_sigma_home_w','meas_sigma_home_m','meas_sigma_market','meas_sigma_home_share_exp_w']

    def allocate(self):
        """ allocate model """

        # a. update grids
        self.setup_grids()

        # b. setup solution: memory allocation
        self.setup_solution()

        # c. setup data
        self.allocate_data()
    
    def setup_grids(self):
        par = self.par

        # for grid search
        par.grid_labor = np.linspace(0.0,par.total_time,par.num_grid)
        par.grid_home = np.linspace(0.0,par.total_time,par.num_grid)
        par.grid_market = np.linspace(0.0,200.0,par.num_grid)

        # Gaussian quadrature for correlated standard normal draws
        nodes, weights = hermgauss(par.num_nodes)
        par.nodes = nodes * np.sqrt(2) # scale nodes for standard normal
        par.weights = weights / np.sqrt(np.pi) # scale weights for standard normal


    def setup_solution(self):
        '''This function allocates memory to store the model solution predictions'''
        # unpack
        par = self.par
        sol = self.sol

        # d. solution arrays
        shape = (par.num_N,)
        sol.labor_w = np.nan + np.zeros(shape)
        sol.labor_m = np.nan + np.zeros(shape)

        sol.home_w = np.nan + np.zeros(shape)
        sol.home_m = np.nan + np.zeros(shape)

        sol.market = np.nan + np.zeros(shape)

        sol.home_share_exp_w = np.nan + np.zeros(shape)

        # initial values (shares)
        sol.init_labor_w = 0.5 + np.zeros(shape)
        sol.init_labor_m = 0.5 + np.zeros(shape)

        sol.init_home_w = 0.1 + np.zeros(shape)
        sol.init_home_m = 0.1 + np.zeros(shape)

        sol.init_market = 0.1 + np.zeros(shape)


    def allocate_data(self):
        '''This function allocates memory to store the data'''
        # unpack
        par = self.par
        sim = self.sim

        # allocate memory for data arrays
        shape = (par.num_N,)
        sim.wage_w = np.nan + np.zeros(shape)
        sim.wage_m = np.nan + np.zeros(shape)

        sim.work_flex_w = np.zeros(shape,dtype=bool) # placeholder, to be replaced by actual data loading
        sim.work_flex_m = np.zeros(shape,dtype=bool) # placeholder, to be replaced by actual data loading

        sim.norm_w = np.nan + np.zeros(shape)
        sim.norm_m = np.nan + np.zeros(shape)

        sim.norm_alt_w = np.nan + np.zeros(shape)
        sim.norm_alt_m = np.nan + np.zeros(shape)
        
        sim.nkids = np.zeros(shape,dtype=np.int64) # placeholder, to be replaced by actual data loading
        
        # choice variables
        sim.labor_w = np.nan + np.zeros(shape)
        sim.labor_m = np.nan + np.zeros(shape)

        sim.home_w = np.nan + np.zeros(shape)
        sim.home_m = np.nan + np.zeros(shape)

        sim.market = np.nan + np.zeros(shape)

        sim.home_share_exp_w = np.nan + np.zeros(shape)


    def load_data(self,data=None,do_simulate=False):
        '''This function loads data and stores it in 'sim'

        Args:
            data (pandas.DataFrame,optional): real data. Column names must match sim variable
                names (see .sim namespace). Not all sim variables need be present (missing ones
                are left at their placeholder values).
            do_simulate (bool,optional): if True, simulate data instead of using `data`
        '''
        # unpack
        par = self.par
        sim = self.sim
        sol = self.sol

        if do_simulate:

            # construct coavriance matrix for wages in order to sample from bivariate normal distribution.
            L11 = np.exp(par.wage_chol1)
            L21 = par.wage_chol2
            L22 = np.exp(par.wage_chol3)

            sigma2_w  = L11*L11
            sigma_wm  = L11*L21
            sigma2_m  = L21*L21 + L22*L22

            par.wage_sigma_w = np.sqrt(sigma2_w)
            par.wage_sigma_m = np.sqrt(sigma2_m)
            par.wage_cov = sigma_wm

            # Generate correlated log wages through independent standard normal shocks
            np.random.seed(par.seed)
            z_w = np.random.normal(size=par.num_N)
            z_m = np.random.normal(size=par.num_N)

            log_wage_w = par.wage_logmean_w + L11 * z_w
            log_wage_m = par.wage_logmean_m + L21 * z_w + L22 * z_m

            # Convert to wage levels
            sim.wage_w[:] = np.exp(log_wage_w)
            sim.wage_m[:] = np.exp(log_wage_m)

            # sim.wage_w[:] = np.linspace(0.1,30.0,par.num_N) # placeholder, to be replaced by actual data loading
            # sim.wage_m[:] = np.linspace(0.1,40.0,par.num_N) # placeholder, to be replaced by actual data loading

            sim.work_flex_w[:] = np.zeros(par.num_N,dtype=bool) # placeholder, to be replaced by actual data loading
            sim.work_flex_m[:] = np.zeros(par.num_N,dtype=bool) # placeholder, to be replaced by actual data loading

            sim.norm_w[:] = 0.5 + np.zeros(par.num_N) # placeholder, to be replaced by actual data loading
            sim.norm_m[:] = 0.5 + np.zeros(par.num_N) # placeholder, to be replaced by actual data loading

            sim.norm_alt_w[:] = sim.norm_w[:]+0.1  # placeholder, to be replaced by actual data loading
            sim.norm_alt_m[:] = 1 - sim.norm_alt_w[:] # placeholder, to be replaced by actual data loading

            # solve model and store solution in simulate
            self.solve(do_exp=True,do_gridsearch=False);

            for var in ('labor_w','labor_m','home_w','home_m','market','home_share_exp_w'):
                # add measurement error to outcomes
                error = getattr(par,'meas_sigma_'+var) * np.random.normal(loc=0.0,scale=1.0,size=par.num_N) 
                obs = getattr(sol,var).copy()
                obs_w_error = np.clip(obs + error,obs.min(),obs.max()) # clip obs at observed min and max
                setattr(sim,var,obs_w_error)

                # reset to nan in sol
                setattr(sol,var,np.nan+np.ones(sol.labor_w.shape))
        
        elif (data is not None):

            # a. resize sol/sim to match the data and reset sol to placeholders
            par.num_N = len(data)
            self.allocate()

            # b. compare data columns to sim variable names
            expected_vars = set(vars(sim).keys())
            data_vars = set(data.columns)

            unmatched_columns = data_vars - expected_vars
            if unmatched_columns:
                print(f'load_data warning: columns {sorted(unmatched_columns)} do not correspond to any sim variable and will be ignored')

            missing_vars = expected_vars - data_vars
            if missing_vars:
                print(f'load_data warning: sim variables {sorted(missing_vars)} were not found in data and remain at their placeholder values')

            # c. copy over matched variables, casting to the dtype allocate() assigned
            for var in expected_vars & data_vars:
                target_dtype = getattr(sim,var).dtype
                setattr(sim,var,np.asarray(data[var].values,dtype=target_dtype))

        else:
            print('load_data: nothing to do, pass do_simulate=True or data=<DataFrame>')

    ############
    # Solution #
    def solve(self,do_exp=False,do_gridsearch=None):

        # a. unpack
        par = self.par
        sol = self.sol
        sim = self.sim

        if do_gridsearch is not None:
            par.do_gridsearch = do_gridsearch

        # b. calculate share of domestic work done by women in experiment. Do this first such that the original outcomes are stored in sol and only sol.home_share_exp_w[:] is based on the alternative.
        if do_exp:
            self.cpp.solve(sim.norm_alt_w,sim.norm_alt_m,sol,sim,par)
            sol.home_share_exp_w[:] = sol.home_w/(sol.home_w + sol.home_m)
            
        # b. call c-functions
        self.cpp.solve(sim.norm_w,sim.norm_m,sol,sim,par)
        

##############
# Estimation #
def initial_wage_parameters(model):
    """
    Construct initial values for the five wage-distribution parameters:
        mu_w, mu_m, kappa_1, kappa_2, kappa_3
    using observations where both spouses work.

    Wage distribution:
        log(w) ~ N(mu, L L')

    with
        L = [[exp(kappa_1), 0],
             [kappa_3,      exp(kappa_2)]]
    """
    # unpack
    sim = model.sim
    wage_w = sim.wage_w
    wage_m = sim.wage_m
    working_w = sim.labor_w > 0
    working_m = sim.labor_m > 0

    # Couples where both spouses work
    both_working = (working_w > 0) & (working_m > 0)

    # Log wages among both-working couples
    log_w = np.log(wage_w[both_working])
    log_m = np.log(wage_m[both_working])

    # Mean parameters
    wage_logmean_w = np.mean(log_w)
    wage_logmean_m = np.mean(log_m)

    # Deviations from means
    dw = log_w - wage_logmean_w
    dm = log_m - wage_logmean_m

    # Covariance matrix (ML version, dividing by N)
    sigma2_w = np.mean(dw*dw)
    sigma2_m = np.mean(dm*dm)
    sigma_wm = np.mean(dw * dm)

    # Cholesky parameters
    sigma_w = np.sqrt(sigma2_w)

    chol1 = np.log(sigma_w)
    chol2 = sigma_wm / sigma_w

    variance_m = sigma2_m - sigma_wm**2 / sigma2_w
    chol3 = np.log(np.sqrt(variance_m))

    return wage_logmean_w, wage_logmean_m, chol1, chol2, chol3




def set_defaults(var_list,weight):
    if var_list is None:
        var_list = ['labor_w','labor_m','home_w','home_m','market','home_share_exp_w'] # M outcome variables
    if weight is None:
        weight = np.eye(len(var_list))
    return var_list,weight

def model_pred(theta,theta_names,model,var_list=None,do_gridsearch=False):
    var_list,weight = set_defaults(var_list,0) # pass value for weight. Does not matter here

    # update parameters
    for i, name in enumerate(theta_names):
        setattr(model.par,name,theta[i])

    # solve model 
    do_exp = True if 'home_share_exp_w' in var_list else False
    model.solve(do_exp=do_exp,do_gridsearch=do_gridsearch)

    # get model predictions
    model_outcomes = np.column_stack([getattr(model.sol,var) for var in var_list])
    
    return model_outcomes

def obj(theta,theta_names,model,data_outcomes=None,weight=None,var_list=None,do_gridsearch=False,do_print=False):
    # set defaults
    var_list,weight = set_defaults(var_list,weight)

    # model predicted outcomes
    model_outcomes = model_pred(theta,theta_names,model,var_list=var_list,do_gridsearch=do_gridsearch)

    # calculate squared difference between model and data (stored in sim)
    if data_outcomes is None:
        data_outcomes = np.column_stack([getattr(model.sim,var) for var in var_list]) 
    
    
    residuals = (data_outcomes - model_outcomes).T # M-by-N matrix of residuals, where M is the number of outcome variables and N is the number of observations

    N = model_outcomes.shape[0]
    mean_sq_residuals = np.trace(residuals.T @ weight @ residuals)/N # this is the mean weighted squared difference across all variables and observations

    if do_print: 
        for i, name in enumerate(theta_names):
            print(f'{name}={theta[i]:2.3f} ',end='')
        print(f'->obj: {mean_sq_residuals:2.4f}')
    return mean_sq_residuals

def loglik_vec(theta,theta_names,model):
    # update parameters
    for i, name in enumerate(theta_names):
        setattr(model.par,name,theta[i])

    # call c-function to calculate log-likelihood
    loglik_contribution = np.nan + np.ones(model.par.num_N)
    model.cpp.LogLik(loglik_contribution, model.sol, model.sim, model.par)

    return loglik_contribution

def obj_loglik(theta,theta_names,model,do_print=False):
    # Calcualte loglikelihood contributions
    loglik_contribution = loglik_vec(theta,theta_names,model)

    loglik = np.mean(loglik_contribution)

    if do_print: 
        for i, name in enumerate(theta_names):
            print(f'{name}={theta[i]:2.3f} ',end='')
        print(f'->obj: {-loglik:2.4f}')

    return -loglik # return negative log-likelihood for minimization


# Standard errors (MLE)
def numerical_derivative_loglik_vec(theta, theta_names, model, eps=1e-5):
    """
    Numerical derivatives of household-level log-likelihood contributions.

    Parameters
    ----------
    theta : array-like, shape (K,)
        Parameter vector.
    theta_names : list
        Parameter names.
    model : object
        Model passed to loglik_vec().
    eps : float
        Relative step size for finite differences.

    Returns
    -------
    score : ndarray, shape (N, K)
        score[i, k] = d log L_i(theta) / d theta_k
    """
    
    # Evaluate once to determine N
    N = len(model.sim.wage_w)  # number of observations
    K = len(theta)

    score = np.zeros((N, K))
    for k in range(K):

        # Parameter-specific step
        h = eps * max(1.0, abs(theta[k]))

        theta_plus = theta.copy()
        theta_minus = theta.copy()

        theta_plus[k] += h
        theta_minus[k] -= h

        ll_plus = loglik_vec(theta_plus, theta_names, model)
        ll_minus = loglik_vec(theta_minus, theta_names, model)

        # Central difference
        score[:, k] = (ll_plus - ll_minus) / (2.0 * h)

    return score

def hessian(theta, theta_names, model, eps=1e-3):
    """
    Calculate the Hessian of the log-likelihood.

    Parameters
    ----------
    theta : array-like, shape (K,)
        Parameter vector.
    theta_names : list
        Parameter names.
    model : object
        Model object.
    eps : float
        Step size for the outer (Hessian) finite difference. Deliberately
        larger than numerical_derivative_loglik_vec's own (inner) step size,
        since the score itself is only known up to numerical/optimizer noise:
        an outer step of the same size would make the double-differenced
        Hessian dominated by that noise rather than the true curvature.

    Returns
    -------
    H : ndarray, shape (K, K)
        Hessian of the log-likelihood.
    """

    K = len(theta)
    H = np.zeros((K, K))
    for k in range(K):

        theta_plus = theta.copy()
        theta_minus = theta.copy()

        h = eps * max(1.0, abs(theta[k]))

        theta_plus[k] += h
        theta_minus[k] -= h

        # N x K matrices of individual scores
        score_plus = numerical_derivative_loglik_vec(theta_plus, theta_names, model)
        score_minus = numerical_derivative_loglik_vec(theta_minus, theta_names, model)

        # Mean score at theta+ and theta-
        sum_score_plus = np.sum(score_plus, axis=0)
        sum_score_minus = np.sum(score_minus, axis=0)

        # Derivative of the sum of score with respect to theta_k
        H[:, k] = (sum_score_plus - sum_score_minus) / (2.0 * h)

    return H

def standard_errors_mle(theta, theta_names, model, clusters=None):
    """
    Calculate standard errors of MLE estimates using the Hessian.

    Parameters
    ----------
    theta : array-like, shape (K,)
        Parameter vector.
    theta_names : list
        Parameter names.
    model : object
        Model object.
    clusters : array-like, shape (N,), optional
        Cluster identifiers for cluster-robust standard errors.

    Returns
    -------
    se_theta : ndarray, shape (K,)
        Standard errors of the parameter estimates.
    """
    
    # Calculate Hessian
    H = -hessian(theta, theta_names, model)
    H = 0.5 * (H + H.T) # ensure symmetry

    # Calculate individual scores
    scores = numerical_derivative_loglik_vec(theta, theta_names, model)

    N,K = scores.shape
    B_meat = np.zeros((K, K))
    if clusters is None:
        B_meat = scores.T @ scores

    else:
        cluster = np.unique(clusters)
        G = len(cluster)
        for g in cluster:

            # Sum individual scores within cluster
            S_g = scores[clusters == g, :].sum(axis=0)

            # Outer product of cluster score
            B_meat += np.outer(S_g, S_g)

        # finite sample/cluster correction        
        correction = (G / (G - 1)) * ((N - 1) / (N - K))
        B_meat *= correction

    # Invert Hessian to get variance-covariance matrix
    A = np.linalg.solve(H, B_meat)
    var_cov_theta = np.linalg.solve(H, A.T).T
    var_cov_theta = 0.5 * (var_cov_theta + var_cov_theta.T) # ensure symmetry

    # Standard errors are the square roots of the diagonal elements
    se_theta = np.sqrt(np.diag(var_cov_theta))

    return se_theta

# Standard errors (NLS)
def Jacobian(theta,theta_names,model,var_list=None,do_gridsearch=False):
    # returns N-by-M-by-K Jacobian matrix, where N is the number of observations, M is the number of outcome variables, and K is the number of parameters to estimate
    var_list,weight = set_defaults(var_list,0) # pass value for weight. Does not matter here

    # model predicted outcomes
    pred = lambda theta: model_pred(theta,theta_names,model,var_list=var_list,do_gridsearch=do_gridsearch)
    model_outcomes = pred(theta)

    # calculate Jacobian matrix using finite (central) differences
    epsilon = 1e-6
    N = model_outcomes.shape[0]
    M = len(var_list)
    K = len(theta_names)
    J = np.zeros((N,M,K))
    for k in range(K):
        theta_plus = theta.copy()
        theta_plus[k] += epsilon
        theta_minus = theta.copy()
        theta_minus[k] -= epsilon

        model_outcomes_plus = pred(theta_plus)
        model_outcomes_minus = pred(theta_minus)

        J[:,:,k] = (model_outcomes_plus - model_outcomes_minus)/(2*epsilon)

    return J

def standard_errors(theta,theta_names,model,data_outcomes=None,weight=None,var_list=None,clusters=None,do_gridsearch=False):
    # returns the standard errors
    # N is the number of observations, M is the number of outcome variables, and K is the number of parameters to estimate

    # set defaults
    var_list,weight = set_defaults(var_list,weight)

    # model predicted outcomes (N-by-M)
    model_outcomes = model_pred(theta,theta_names,model,var_list=var_list,do_gridsearch=do_gridsearch)

    # calculate residuals (N-by-M)
    if data_outcomes is None:
        data_outcomes = np.column_stack([getattr(model.sim,var) for var in var_list]) 
    residuals = data_outcomes - model_outcomes

    # dimensions
    N = model_outcomes.shape[0]
    M = len(var_list)
    K = len(theta_names)

    # calculate Jacobian matrix (N-by-M-by-K)
    J = Jacobian(theta,theta_names,model,var_list=var_list,do_gridsearch=do_gridsearch)

    # Calculate A and individual score contributions
    A = np.zeros((K, K))
    scores = np.zeros((N, K))

    for i in range(N):
        Di = J[i, :, :]                       # M x K
        ei = residuals[i, :].reshape(M, 1)    # M x 1

        # A
        A += Di.T @ weight @ Di

        # Individual score contribution
        scores[i, :] = (Di.T @ weight @ ei).ravel()

    A /= N

    # Cluster-robust B
    B = np.zeros((K, K))
    if clusters is not None:
        unique_clusters = np.unique(clusters)
        G = len(unique_clusters)

        for g in unique_clusters:
            cluster_scores = scores[clusters == g, :]
            Sg = cluster_scores.sum(axis=0).reshape(K, 1)
            B += Sg @ Sg.T
    else:
        for i in range(N):
            si = scores[i, :].reshape(K, 1)
            B += si @ si.T

    B /= N

    # calculate variance-covariance matrix of the parameter estimates using the sandwich formula
    A_inv_B = np.linalg.solve(A, B)
    var_cov_theta = np.linalg.solve(A, A_inv_B.T).T / N
    
    se_theta = np.sqrt(np.diag(var_cov_theta))
    return se_theta