
#define MAIN
#include "myheader.h"

// include these again here to ensure that they are automatically compiled by consav
#ifndef MAIN
#include "utils.cpp"
#endif


EXPORT void solve(double* norm_w, double* norm_m, sol_struct *sol,sim_struct *sim, par_struct *par){
    // this is the main solution function called from Python
    #pragma omp parallel num_threads(par->cpp_threads) shared(sol,sim,par)
    {
        
        // loop through observations
        #pragma omp for
        for (int i = 0; i < par->num_N; i++){

            sol->init_labor_w[i] = 0.5; // 0ld:0.7
            sol->init_labor_m[i] = 0.5; // old:0.7
            sol->init_home_w[i] = 0.1; //old: 0.1
            sol->init_home_m[i] = 0.1; //old: 0.1
            sol->init_market[i] = 0.1; //old: 0.1
            
            // solve and predict outcomes for this observation
            utils::solve_predict(sim->wage_w[i], sim->wage_m[i], norm_w[i], norm_m[i], i, sol, sim, par);

            // again with the found optimum as starting values
            // utils::solve_predict(i, sol, sim, par);

        }

    }
}

const char* var_list[] = {
    "labor_w",
    "labor_m",
    "home_w",
    "home_m",
    "market",
    "home_share_exp_w" 
};
const int num_var = 6;

void solve_with_experiment(double wage_w, double wage_m, int i, sol_struct *sol, sim_struct *sim, par_struct *par){
    // this function solves the model for a given observation with a given wage offer and predicts outcomes. It is used for counterfactual experiments.

    // first do experiement
    utils::solve_predict(wage_w, wage_m, sim->norm_alt_w[i], sim->norm_alt_m[i], i, sol, sim, par);
    sol->home_share_exp_w[i] = sol->home_w[i]/(sol->home_w[i]+sol->home_m[i]);

    // restore and solve baseline
    utils::solve_predict(wage_w, wage_m, sim->norm_w[i], sim->norm_m[i], i, sol, sim, par);

}

double log_lognormal_pdf(double logwage, double logmean, double sd)
{
    double a = (logwage - logmean) / sd;

    return -halflog2pi - logwage - std::log(sd) - 0.5 * a * a; // notice that we devide by wage here (giving -x) to get the correct pdf for the lognormal distribution
}

double log_bivariate_lognormal_pdf(double wage_w,double wage_m,double mu_w,double mu_m,double sigma2_w,double sigma2_m,double sigma_wm){
    double x = std::log(wage_w);
    double y = std::log(wage_m);

    double dx = x - mu_w;
    double dy = y - mu_m;

    double det = sigma2_w*sigma2_m - sigma_wm*sigma_wm;

    double quad = ( sigma2_m*dx*dx - 2.0*sigma_wm*dx*dy + sigma2_w*dy*dy ) / det;

    return -2.0*halflog2pi - x - y - 0.5*std::log(det) -0.5*quad;
}

double loglik_outcomes(int i, sol_struct *sol, sim_struct *sim, par_struct *par){
    double loglik = 0.0;
    
    // likelihood contribution from outcomes (assuming independence across outcomes). This is the same across all versions so put in function
    for (int m = 0; m < num_var; m++) {
        double outcome = get_double_p_sim_struct(sim,const_cast<char*>(var_list[m]))[i];
        double predict = get_double_p_sol_struct(sol,const_cast<char*>(var_list[m]))[i];
        double residual = outcome - predict;

        double sigma = 0.0;
        if(par->do_meas_sigma_profile){
            sigma = par->meas_sigma_vec[m]; // use the profile value of the measurement error standard deviation for this outcome
        } else {
            std::string var_name = "meas_sigma_" + std::string(var_list[m]);
            sigma = get_double_par_struct(par, const_cast<char*>(var_name.c_str())); // variance of the measurement error related to each outcome.
        }
        double var = sigma*sigma;
        // compute likelihood contribution from residual (assuming normality). log -var can be done once
        loglik += -0.5 * (std::log(var) + residual*residual/var);

    }
    loglik -= num_var * halflog2pi; // constant term for normal distribution. can be done once
    
    return loglik;
}

EXPORT void LogLik(double* LogLik_vec, sol_struct *sol,sim_struct *sim, par_struct *par){
    
    // lower cholesky decomposition of wage covariance used for Gaussian quadrature. Parameterize that rather than the covariance to ensure positive definiteness.
    double L11 = std::exp(par->wage_chol1);
    double L21 = par->wage_chol2;
    double L22 = std::exp(par->wage_chol3);

    double sigma2_w  = L11*L11;
    double sigma_wm  = L11*L21;
    double sigma2_m  = L21*L21 + L22*L22;

    double sigma_w = std::sqrt(sigma2_w);
    double sigma_m = std::sqrt(sigma2_m);

    // conditional standard normal stamdard deviations. Used in the cases where one partner works. 
    double sigma_w_cond = std::sqrt(sigma2_w - sigma_wm*sigma_wm/(sigma2_m));
    double sigma_m_cond = std::sqrt(sigma2_m - sigma_wm*sigma_wm/(sigma2_w));

    // First, solve the model for all couples in which both spouses are working. This is then used to estimate the measurement error variances.
    double sum1 = 0.0, sum2=0.0, sum3=0.0, sum4=0.0, sum5=0.0, sum6=0.0;
    int numN11 = 0; // number of couples where both work
    #pragma omp parallel num_threads(par->cpp_threads) shared(sol,sim,par)
    {

        // loop through observations and solve for both-working cases. This is then used to estimate the measurement error variances.
        #pragma omp for reduction(+:sum1,sum2,sum3,sum4,sum5,sum6,numN11) schedule(static)
        for (int i = 0; i < par->num_N; i++){
            // determine labor supply combination
            bool both_working = (sim->labor_w[i]>0.0) && (sim->labor_m[i]>0.0);
            
            sol->init_labor_w[i] = 0.5; // 0ld:0.7
            sol->init_labor_m[i] = 0.5; // old:0.7
            sol->init_home_w[i] = 0.1; //old: 0.1
            sol->init_home_m[i] = 0.1; //old: 0.1
            sol->init_market[i] = 0.1; //old: 0.1

            if(both_working){
                double wage_w = sim->wage_w[i];
                double wage_m = sim->wage_m[i];

                // solve model with observed wages (these are stored in the solution arrays and should thus not be redone below)
                solve_with_experiment(wage_w, wage_m, i, sol, sim, par);
                
                // add to sum of squared differences for all 6 outcomes
                double diff1 = sim->labor_w[i] - sol->labor_w[i];
                double diff2 = sim->labor_m[i] - sol->labor_m[i];
                double diff3 = sim->home_w[i] - sol->home_w[i];
                double diff4 = sim->home_m[i] - sol->home_m[i];
                double diff5 = sim->market[i] - sol->market[i];
                double diff6 = sim->home_share_exp_w[i] - sol->home_share_exp_w[i];

                sum1 += diff1*diff1;
                sum2 += diff2*diff2;
                sum3 += diff3*diff3;
                sum4 += diff4*diff4;
                sum5 += diff5*diff5;
                sum6 += diff6*diff6;

                numN11 ++;
            }
        }
    } // pragma
    
    // store estimated variances (std) [NOTE: order important! must follow the var_list order above]
    if(numN11 > 0){
        par->meas_sigma_vec[0] = std::sqrt(sum1/numN11);
        par->meas_sigma_vec[1] = std::sqrt(sum2/numN11);
        par->meas_sigma_vec[2] = std::sqrt(sum3/numN11);
        par->meas_sigma_vec[3] = std::sqrt(sum4/numN11);
        par->meas_sigma_vec[4] = std::sqrt(sum5/numN11);
        par->meas_sigma_vec[5] = std::sqrt(sum6/numN11);
    } else {
        for(int m = 0; m < num_var; m++){
            par->meas_sigma_vec[m] = NAN;
        }
    }

    // Now, solve the model for all couples and compute the likelihood contribution for each observation. This is done in parallel across observations.
    #pragma omp parallel num_threads(par->cpp_threads) shared(sol,sim,par)
    {
        // loop through observations
        #pragma omp for
        for (int i = 0; i < par->num_N; i++){
            // determine labor supply combination
            bool both_working = (sim->labor_w[i]>0.0) && (sim->labor_m[i]>0.0);
            bool woman_working = (sim->labor_w[i]>0.0) && (sim->labor_m[i]==0.0);
            bool man_working = (sim->labor_w[i]==0.0) && (sim->labor_m[i]>0.0);
            bool none_working = (sim->labor_w[i]==0.0) && (sim->labor_m[i]==0.0);

            double Elik_out = 0.0;
            double logElik_out = 0.0; // this is to calculate the likelihood contribution related to observed outcomes. If one works, this is the marginal pdf evaluated at the observed outcome.
            double loglik_wage_obs = 0.0; // this is to calculate the likelihood contribution related to observed wages. If one works, this is the marginal pdf evaluated at the observed wage.

            sol->init_labor_w[i] = 0.5; // 0ld:0.7
            sol->init_labor_m[i] = 0.5; // old:0.7
            sol->init_home_w[i] = 0.1; //old: 0.1
            sol->init_home_m[i] = 0.1; //old: 0.1
            sol->init_market[i] = 0.1; //old: 0.1

            if(both_working){
                double wage_w = sim->wage_w[i];
                double wage_m = sim->wage_m[i];

                // log-likelihood contribution from wage observations of women and men (bivariate lognormal)
                loglik_wage_obs = log_bivariate_lognormal_pdf(wage_w,wage_m,par->wage_logmean_w,par->wage_logmean_m,sigma2_w,sigma2_m,sigma_wm);

                // solve model with observed wages (this has already been done above)
                // solve_with_experiment(wage_w, wage_m, i, sol, sim, par);
                
                // likelihood contribution from outcomes (assuming independence across outcomes). 
                logElik_out = loglik_outcomes(i, sol, sim, par);

            
            } else if (woman_working){
                // observed female wage
                double wage_w = sim->wage_w[i];
                double logwage_w = std::log(wage_w);

                // log-likelihood contribution from wage observation of women (one-dimensional lognormal for women)
                loglik_wage_obs = log_lognormal_pdf(logwage_w,par->wage_logmean_w,sigma_w); // wage distribution (one-dimensional lognormal for women)

                // conditional distribution of male wage given female wage
                double mu_m_cond = par->wage_logmean_m + sigma_wm/sigma2_w * (logwage_w - par->wage_logmean_w);

                // loop through all wage draws for men
                for(int im=0; im<par->num_nodes; im++){
                    // get standard normal draw
                    double node_m = par->nodes[im];
                    double weight = par->weights[im];

                    // draw male wage offer
                    double wage_m = std::exp(mu_m_cond + sigma_m_cond * node_m);

                    // solve model with these draws
                    solve_with_experiment(wage_w, wage_m, i, sol, sim, par);

                    // likelihood contribution from outcomes (assuming independence across outcomes).
                    Elik_out += weight * std::exp(loglik_outcomes(i, sol, sim, par)); // integrate over the wage distribution for men. weight here represents the conditional density of the male wage given the female wage
                    
                }
            
            } else if (man_working){
                // observed male wages
                double wage_m = sim->wage_m[i];
                double logwage_m = std::log(wage_m);

                // log-likelihood contribution from wage observation of men (one-dimensional lognormal for men)
                loglik_wage_obs = log_lognormal_pdf(logwage_m,par->wage_logmean_m,sigma_m); 

                // conditional distribution of female wage given male wage
                double mu_w_cond = par->wage_logmean_w + sigma_wm/(sigma2_m) * (logwage_m - par->wage_logmean_m);

                // loop through all wage draws for women
                for(int iw=0; iw<par->num_nodes;iw++){
                    // get standard normal draws
                    double node_w = par->nodes[iw];
                    double weight = par->weights[iw];

                    // wages
                    double wage_w = std::exp(mu_w_cond + sigma_w_cond * node_w);

                    // solve model with these draws
                    solve_with_experiment(wage_w, wage_m, i, sol, sim, par);
                    
                    // likelihood contribution from outcomes (assuming independence across outcomes). 
                    Elik_out += weight * std::exp(loglik_outcomes(i, sol, sim, par)); // integrate over the wage distribution for women. weight here represents the conditional density of the female wage given the male wage
                    
                }                
            
            } else if (none_working){
                // wage distribution 0=log(1) since we integrate over the wage distribution for both men and women so that is the joint distribution of wages.
                loglik_wage_obs = 0.0; 
                
                for(int iw=0; iw<par->num_nodes;iw++){
                    for(int im=0; im<par->num_nodes;im++){
                        // get standard normal draws
                        double node_w = par->nodes[iw];
                        double node_m = par->nodes[im];
                        double weight = par->weights[iw] * par->weights[im];

                        // Correlated standard normal draws
                        double z_w = L11 * node_w;
                        double z_m = L21 * node_w + L22 * node_m;

                        // wages
                        double wage_w = std::exp(par->wage_logmean_w + z_w);
                        double wage_m = std::exp(par->wage_logmean_m + z_m);

                        // solve for this wage draw
                        solve_with_experiment(wage_w, wage_m, i, sol, sim, par);
                        
                        // likelihood contribution from outcomes (assuming independence across outcomes). This is the same across all versions so put in function
                        Elik_out += weight * std::exp(loglik_outcomes(i, sol, sim, par)); 
                        
                    }
                }
                
            } else {
                Elik_out = 9210.0; // should never end here
                loglik_wage_obs = 0.0;
            }

            if(!both_working){
                // log of expected likelihood
                if(Elik_out>0.0){
                    logElik_out = std::log(Elik_out);
                } else {
                    logElik_out = -1e10; 
                }
            }

            // store likelihood contribution for this observation
            LogLik_vec[i] = loglik_wage_obs + logElik_out;
        }

    }
}