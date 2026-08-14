// functions related to utility and environment.
#ifndef MAIN
#define UTILS
#include "myheader.cpp"
#endif

namespace utils {

    double bargaining_weight(int i, sim_struct* sim, par_struct* par){
        double w = 0.5; // placeholder
        return w;
    }

    double norm_penalty(double norm_diff, int gender, int i, sim_struct* sim, par_struct* par){
        double norm_pos = par->norm_pos_w;
        double norm_neg = par->norm_neg_w;
        double norm_close = par->norm_close_w;
        double norm_attract = par->norm_attract_w;
        if (gender==man){
            norm_pos = par->norm_pos_m;
            norm_neg = par->norm_neg_m;
            norm_close = par->norm_close_m;
            norm_attract = par->norm_attract_m;
        }

        if(std::abs(norm_diff) < norm_close){
            return norm_attract;

        } else if (norm_diff > 0.0){
            return norm_pos * norm_diff*norm_diff;

        } else {
            return norm_neg * norm_diff*norm_diff;
            
        }

    }

    double home_production(double home_w, double home_m,int i, sim_struct* sim, par_struct* par){
        double effective_home_w = sim->work_flex_w[i] ? home_w*par->home_effective_w : home_w;
        double effective_home_m = sim->work_flex_m[i] ? home_m*par->home_effective_m : home_m;
        
        double home_production_w = par->home_weight*pow(effective_home_w, par->home_power);
        double home_production_m = (2.0-par->home_weight)*pow(effective_home_m, par->home_power);
        
        double home = pow(home_production_w + home_production_m, 1.0/par->home_power);

        return home;
    }

    double public_good(double home_w, double home_m, double market,int i, sim_struct* sim, par_struct* par){
        double market_purchase = par->public_weight*pow(market, par->public_power);

        double H = home_production(home_w,home_m,i,sim,par);
        double home_produced = (1.0-par->public_weight)*pow(H, par->public_power);
        
        double public_consumption = pow(market_purchase + home_produced, 1.0/par->public_power);
        return public_consumption;
    }

    double income(double labor_w, double labor_m, double wage_w, double wage_m, int i, sim_struct* sim, par_struct* par){
        double income_w = labor_w*wage_w;
        double income_m = labor_m*wage_m;

        double transfers = (labor_w==0.0)*par->unemployment + (labor_m==0.0)*par->unemployment;
        double taxes = (income_w + income_m + transfers) * par->tax;

        double income = income_w + income_m + transfers - taxes;
        return income;
    }

    double util_couple(double labor_w, double labor_m, double home_w, double home_m, double market, double wage_w, double wage_m, double norm_w, double norm_m, int i, sim_struct* sim, par_struct* par){
        
        // consumption goods
        double consumption = income(labor_w, labor_m, wage_w, wage_m, i, sim, par) - market;
        double util_consumption = pow(consumption,1.0-par->crra)/(1.0-par->crra);

        double public_consumption = public_good(home_w, home_m, market, i, sim, par);
        double util_public = par->util_public*log(public_consumption);

        // leisure 
        double leisure_w = par->total_time - labor_w - home_w;
        double leisure_m = par->total_time - labor_m - home_m;

        double util_leisure_w = par->util_leisure_w*pow(leisure_w, 1.0 - par->leisure_power_w)/(1.0 - par->leisure_power_w);
        double util_leisure_m = par->util_leisure_m*pow(leisure_m, 1.0 - par->leisure_power_m)/(1.0 - par->leisure_power_m);
        
        // norms
        double share_home_w = home_w/(home_w + home_m);
        double norm_diff_w = share_home_w - norm_w;
        double norm_diff_m = (1.0 - share_home_w) - norm_m;

        double util_norm_w = norm_penalty(norm_diff_w, woman, i, sim, par);
        double util_norm_m = norm_penalty(norm_diff_m, man, i, sim, par);

        // household utility: weighted individual utility
        double util_w = util_consumption + util_public + util_leisure_w + util_norm_w;
        double util_m = util_consumption + util_public + util_leisure_m + util_norm_m;

        double power_weight = bargaining_weight(i,sim,par);
        double util_couple = power_weight*util_w + (1.0-power_weight)*util_m;
        
        return util_couple;
    }



// Solving the model
void setup_nlopt(nlopt_opt opt,int dim){
        // bounds (all shares)
        double* lb = new double[dim];
        double* ub = new double[dim];

        for(int d=0;d<dim;d++){
            lb[d] = 0.0001;
            ub[d] = 0.9999;
        }
        nlopt_set_lower_bounds(opt, lb);
        nlopt_set_upper_bounds(opt, ub);

        // stopping criteria
        nlopt_set_maxeval(opt, 500);       
        nlopt_set_ftol_rel(opt, 1e-8);
        nlopt_set_xtol_rel(opt, 1e-6);
        
        // delete memory
        delete[] lb;
        delete[] ub;
    }

double penalty_clip(double* share,double min=0.0, double max=1.0){
    double penalty = 0.0; // penalty will be negative if outside bounds
    if (share[0]<min){
        penalty += (share[0] - min) * 1000.0;
        share[0] = min;
    }

    if (share[0]>max){
        penalty += (max - share[0]) * 1000.0;
        share[0] = max;
    }

    return penalty;
}

double market_level(double labor_w, double labor_m, double wage_w, double wage_m, double market_share, int i, sim_struct *sim, par_struct *par){
    return market_share * income(labor_w,labor_m,wage_w,wage_m,i,sim,par);
}
double home_level(double labor, double home_share,par_struct *par){
    return home_share * (par->total_time - labor);
}
double labor_level(double labor_share,par_struct *par){
    return labor_share * par->total_time;
}

// inner solver for home production and market purchases conditional on labor supply
typedef struct {
        int i;     
        double wage_w;
        double wage_m;
        double norm_w;
        double norm_m;
        double labor_w;
        double labor_m;

        sim_struct *sim;
        par_struct *par;

    } solver_struct_home;

    double objfunc_home(unsigned n, const double *x, double *grad, void *solver_data_in){
        // unpack
        solver_struct_home *solver_data = (solver_struct_home *) solver_data_in;

        int i = solver_data->i;

        double wage_w = solver_data->wage_w;
        double wage_m = solver_data->wage_m;
        double norm_w = solver_data->norm_w;
        double norm_m = solver_data->norm_m;

        double labor_w = solver_data->labor_w;
        double labor_m = solver_data->labor_m;
        
        sim_struct *sim = solver_data->sim;
        par_struct *par = solver_data->par;

        double home_share_w = x[0];
        double home_share_m = x[1];
        double market_share = x[2];
        
        // penalty and clip
        double penalty = 0.0;
        penalty += penalty_clip(&home_share_w,0.0,1.0);
        penalty += penalty_clip(&home_share_m,0.0,1.0);
        penalty += penalty_clip(&market_share,0.0,1.0);

        // re-scale 
        double home_w = home_level(labor_w, home_share_w, par); //share of remaining time -> imposes time constraint
        double home_m = home_level(labor_m, home_share_m, par);
        double market = market_level(labor_w, labor_m, wage_w, wage_m, market_share, i, sim, par);
        
        // utility
        double util = util_couple(labor_w,labor_m,home_w,home_m,market,wage_w,wage_m,norm_w,norm_m,i,sim,par);

        return -(util + penalty);

    }

    typedef struct {
        int i;              
        double wage_w;
        double wage_m;
        double norm_w;
        double norm_m;

        int constraint=0; // 0: no constraint, 1: only women, 2: only men, 3: both not working
        
        sol_struct *sol;
        sim_struct *sim;
        par_struct *par;

        // initial values for inner optimization over home production and market purchases
        double init_home_w = 0.1;
        double init_home_m = 0.1;
        double init_market = 0.1;

    } solver_struct;

    double objfunc(unsigned n, const double *x, double *grad, void *solver_data_in){
        // this objective function takes labor supply of both members as given and searches over home production and market purchases. In shares again.
        // unpack
        solver_struct *solver_data = (solver_struct *) solver_data_in;

        int i = solver_data->i;

        double wage_w = solver_data->wage_w;
        double wage_m = solver_data->wage_m;
        double norm_w = solver_data->norm_w;
        double norm_m = solver_data->norm_m;

        sol_struct *sol = solver_data->sol;
        sim_struct *sim = solver_data->sim;
        par_struct *par = solver_data->par;

        int constraint = solver_data->constraint;

        double labor_share_w=0.0, labor_share_m=0.0;
        if (constraint==woman){
            labor_share_w = x[0];
        } else if (constraint==man){
            labor_share_m = x[0];
        } else if (constraint==3){
            labor_share_w = 0.0;
            labor_share_m = 0.0;
        } else {
            labor_share_w = x[0];
            labor_share_m = x[1];
        }

        // clip and penalty
        double penalty = 0.0;
        penalty += penalty_clip(&labor_share_w,0.0,1.0);
        penalty += penalty_clip(&labor_share_m,0.0,1.0);


        double labor_w = labor_level(labor_share_w, par); // share of total time -> impose constraints 
        double labor_m = labor_level(labor_share_m, par); // share of total time -> impose constraints 
        
        // setup nlopt for inner optimization over home production and market purchases
        int const dim = 3;
        double x_home[dim];
        double minf = 0.0;
        
        solver_struct_home* solver_data_home = new solver_struct_home;
        solver_data_home->i = i; 
        
        solver_data_home->wage_w = wage_w;
        solver_data_home->wage_m = wage_m;
        solver_data_home->norm_w = norm_w;
        solver_data_home->norm_m = norm_m;

        solver_data_home->labor_w = labor_w;
        solver_data_home->labor_m = labor_m;

        solver_data_home->sim = sim;
        solver_data_home->par = par;

        auto opt = nlopt_create(NLOPT_LN_BOBYQA, dim); //NLOPT_LN_BOBYQA NLOPT_LD_MMA NLOPT_LD_LBFGS NLOPT_GN_ORIG_DIRECT
        nlopt_set_min_objective(opt, objfunc_home, solver_data_home);
        setup_nlopt(opt,dim);     

        // optimize
        if(par->reuse_init){
            x_home[0] = solver_data->init_home_w;
            x_home[1] = solver_data->init_home_m;
            x_home[2] = solver_data->init_market;
        } else {
            x_home[0] = 0.1;
            x_home[1] = 0.1;
            x_home[2] = 0.1;
        }
        nlopt_optimize(opt, x_home, &minf);

        // update initial values in solver data for next iteration. These are also used to calculate levels of home production and market purchases in the outer optimization over labor supply.:
        solver_data->init_home_w = x_home[0];
        solver_data->init_home_m = x_home[1];
        solver_data->init_market = x_home[2];

        // delete memory
        nlopt_destroy(opt);
        delete solver_data_home;

        // return objective function (including penalty)
        return minf - penalty;
       
    }

    void numerical_optimizer(double wage_w,double wage_m,double norm_w,double norm_m,int i,sol_struct* sol,sim_struct* sim, par_struct* par){
        // setup NLOPT optimizer over labor supply. Everything is in shares. See how levels are calculated when storing results.
        int const dim = 2;
        double x[dim];
        double minf = 0.0;
        
        solver_struct* solver_data = new solver_struct;
        solver_data->i = i;
        solver_data->wage_w = wage_w;
        solver_data->wage_m = wage_m;
        solver_data->norm_w = norm_w;
        solver_data->norm_m = norm_m;
        
        solver_data->sol = sol;
        solver_data->sim = sim;
        solver_data->par = par;
        
        // initial values for inner optimization over home production and market purchases 
        solver_data->init_home_w = sol->init_home_w[i];
        solver_data->init_home_m = sol->init_home_m[i];
        solver_data->init_market = sol->init_market[i];

        auto opt = nlopt_create(NLOPT_LN_BOBYQA, dim); //NLOPT_LN_BOBYQA NLOPT_LD_MMA NLOPT_LD_LBFGS NLOPT_GN_ORIG_DIRECT
        nlopt_set_min_objective(opt, objfunc, solver_data);
        setup_nlopt(opt,dim);     

        // optimize unconstrained
        solver_data->constraint = 0;
        if(par->reuse_init){
            x[0] = sol->init_labor_w[i];
            x[1] = sol->init_labor_m[i];
        } else {
            x[0] = 0.5;
            x[1] = 0.5;
        } 
        nlopt_optimize(opt, x, &minf); 

        ////////////////////////////
        // check corner solutions //
        double init_home_w = solver_data->init_home_w;
        double init_home_m = solver_data->init_home_m;
        double init_market = solver_data->init_market;

        auto opt_constraint = nlopt_create(NLOPT_LN_BOBYQA, 1); //NLOPT_LN_BOBYQA NLOPT_LD_MMA NLOPT_LD_LBFGS NLOPT_GN_ORIG_DIRECT
        double minf_constraint = 0.0;
        double x_constraint[1];
        
        // only women working
        solver_data->constraint = woman;
        nlopt_set_min_objective(opt_constraint, objfunc, solver_data);
        setup_nlopt(opt_constraint,1);

        x_constraint[0] = 0.5;
        nlopt_optimize(opt_constraint, x_constraint, &minf_constraint);

        if(minf_constraint < minf){
            minf = minf_constraint;
            x[0] = x_constraint[0];
            x[1] = 0.0;

            init_home_w = solver_data->init_home_w;
            init_home_m = solver_data->init_home_m;
            init_market = solver_data->init_market;
        }

        // only men working
        solver_data->constraint = man;
        nlopt_set_min_objective(opt_constraint, objfunc, solver_data);
        setup_nlopt(opt_constraint,1);

        x_constraint[0] = 0.5;
        nlopt_optimize(opt_constraint, x_constraint, &minf_constraint);
        
        if(minf_constraint < minf){
            minf = minf_constraint;
            x[0] = 0.0;
            x[1] = x_constraint[0];

            init_home_w = solver_data->init_home_w;
            init_home_m = solver_data->init_home_m;
            init_market = solver_data->init_market;
        }

        // none working
        solver_data->constraint = 3;
        double util_unemp = objfunc(1, x_constraint, nullptr, solver_data);
        if(util_unemp<minf){
            minf = util_unemp;
            x[0] = 0.0;
            x[1] = 0.0;

            init_home_w = solver_data->init_home_w;
            init_home_m = solver_data->init_home_m;
            init_market = solver_data->init_market;
        }

        // store initial values (shares) for next iteration
        sol->init_labor_w[i] = x[0];
        sol->init_labor_m[i] = x[1];
        sol->init_home_w[i] = init_home_w;
        sol->init_home_m[i] = init_home_m;
        sol->init_market[i] = init_market;

        // store results in levels
        sol->labor_w[i] = labor_level(x[0], par); 
        sol->labor_m[i] = labor_level(x[1], par);
        sol->home_w[i] = home_level(sol->labor_w[i], sol->init_home_w[i], par);
        sol->home_m[i] = home_level(sol->labor_m[i], sol->init_home_m[i], par);
        sol->market[i] = market_level(sol->labor_w[i], sol->labor_m[i], wage_w, wage_m, sol->init_market[i], i, sim, par);

        // delete memory
        nlopt_destroy(opt);
        nlopt_destroy(opt_constraint);
        delete solver_data;
    }


    void solve_predict(double wage_w,double wage_m,double norm_w,double norm_m,int i,sol_struct* sol,sim_struct* sim, par_struct* par){
        if (par->do_gridsearch){
            double obj_max = -HUGE_VAL;
            double obj_now = 0.0;

            double opt_labor_w = -1.0;
            double opt_labor_m = -1.0;
            double opt_home_w = -1.0;
            double opt_home_m = -1.0;
            double opt_market = -1.0;
            

            int num_grid = par->num_grid;
            for(int i_labor_w=0;i_labor_w<num_grid;i_labor_w++){
                double labor_w = par->grid_labor[i_labor_w];
                for(int i_labor_m=0;i_labor_m<num_grid;i_labor_m++){
                    double labor_m = par->grid_labor[i_labor_m];
                    for(int i_home_w=0;i_home_w<num_grid;i_home_w++){
                        double home_w = par->grid_home[i_home_w];
                        for(int i_home_m=0;i_home_m<num_grid;i_home_m++){
                            double home_m = par->grid_home[i_home_m];
                            for(int i_market=0;i_market<num_grid;i_market++){
                                double market = par->grid_market[i_market];

                                double leisure_w = par->total_time - labor_w - home_w;
                                double leisure_m = par->total_time - labor_m - home_m;

                                double consumption = income(labor_w, labor_m,wage_w,wage_m,i,sim,par) - market;
                                
                                if((leisure_w>0.0)&(leisure_m>0.0)&(consumption>0.0)){
                                    obj_now = util_couple(labor_w,labor_m,home_w,home_m,market,wage_w,wage_m,norm_w,norm_m,i,sim,par);
                                
                                    if(obj_now>obj_max){
                                        obj_max = obj_now;

                                        opt_labor_w = labor_w;
                                        opt_labor_m = labor_m;
                                        opt_home_w = home_w;
                                        opt_home_m = home_m;
                                        opt_market = market;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            
            sol->labor_w[i]= opt_labor_w;
            sol->labor_m[i]= opt_labor_m;
            sol->home_w[i]= opt_home_w;
            sol->home_m[i]= opt_home_m;
            sol->market[i]= opt_market;

        } else {
            
            numerical_optimizer(wage_w,wage_m,norm_w,norm_m,i,sol,sim,par); 
        
        }
    }

    
} // utils