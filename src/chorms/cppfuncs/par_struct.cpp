typedef struct par_struct
{
 double total_time;
 double crra;
 double util_public;
 double util_leisure_w;
 double util_leisure_m;
 double leisure_power_w;
 double leisure_power_m;
 double home_effective_w;
 double home_effective_m;
 double home_weight;
 double home_power;
 double public_weight;
 double public_power;
 double norm_pos_w;
 double norm_pos_m;
 double norm_neg_w;
 double norm_neg_m;
 double norm_close_w;
 double norm_close_m;
 double norm_attract_w;
 double norm_attract_m;
 double unemployment;
 double tax;
 double wage_logmean_w;
 double wage_logmean_m;
 int num_nodes;
 double wage_chol1;
 double wage_chol2;
 double wage_chol3;
 double wage_sigma_w;
 double wage_sigma_m;
 double wage_cov;
 double meas_sigma_labor_w;
 double meas_sigma_labor_m;
 double meas_sigma_home_w;
 double meas_sigma_home_m;
 double meas_sigma_market;
 double meas_sigma_home_share_exp_w;
 double* meas_sigma_vec;
 int cpp_threads;
 bool reuse_init;
 bool do_gridsearch;
 int num_grid;
 int num_N;
 int seed;
 bool do_meas_sigma_profile;
 double* grid_labor;
 double* grid_home;
 double* grid_market;
 double* nodes;
 double* weights;
} par_struct;

double get_double_par_struct(par_struct* x, char* name){

 if( strcmp(name,"total_time") == 0 ){ return x->total_time; }
 else if( strcmp(name,"crra") == 0 ){ return x->crra; }
 else if( strcmp(name,"util_public") == 0 ){ return x->util_public; }
 else if( strcmp(name,"util_leisure_w") == 0 ){ return x->util_leisure_w; }
 else if( strcmp(name,"util_leisure_m") == 0 ){ return x->util_leisure_m; }
 else if( strcmp(name,"leisure_power_w") == 0 ){ return x->leisure_power_w; }
 else if( strcmp(name,"leisure_power_m") == 0 ){ return x->leisure_power_m; }
 else if( strcmp(name,"home_effective_w") == 0 ){ return x->home_effective_w; }
 else if( strcmp(name,"home_effective_m") == 0 ){ return x->home_effective_m; }
 else if( strcmp(name,"home_weight") == 0 ){ return x->home_weight; }
 else if( strcmp(name,"home_power") == 0 ){ return x->home_power; }
 else if( strcmp(name,"public_weight") == 0 ){ return x->public_weight; }
 else if( strcmp(name,"public_power") == 0 ){ return x->public_power; }
 else if( strcmp(name,"norm_pos_w") == 0 ){ return x->norm_pos_w; }
 else if( strcmp(name,"norm_pos_m") == 0 ){ return x->norm_pos_m; }
 else if( strcmp(name,"norm_neg_w") == 0 ){ return x->norm_neg_w; }
 else if( strcmp(name,"norm_neg_m") == 0 ){ return x->norm_neg_m; }
 else if( strcmp(name,"norm_close_w") == 0 ){ return x->norm_close_w; }
 else if( strcmp(name,"norm_close_m") == 0 ){ return x->norm_close_m; }
 else if( strcmp(name,"norm_attract_w") == 0 ){ return x->norm_attract_w; }
 else if( strcmp(name,"norm_attract_m") == 0 ){ return x->norm_attract_m; }
 else if( strcmp(name,"unemployment") == 0 ){ return x->unemployment; }
 else if( strcmp(name,"tax") == 0 ){ return x->tax; }
 else if( strcmp(name,"wage_logmean_w") == 0 ){ return x->wage_logmean_w; }
 else if( strcmp(name,"wage_logmean_m") == 0 ){ return x->wage_logmean_m; }
 else if( strcmp(name,"wage_chol1") == 0 ){ return x->wage_chol1; }
 else if( strcmp(name,"wage_chol2") == 0 ){ return x->wage_chol2; }
 else if( strcmp(name,"wage_chol3") == 0 ){ return x->wage_chol3; }
 else if( strcmp(name,"wage_sigma_w") == 0 ){ return x->wage_sigma_w; }
 else if( strcmp(name,"wage_sigma_m") == 0 ){ return x->wage_sigma_m; }
 else if( strcmp(name,"wage_cov") == 0 ){ return x->wage_cov; }
 else if( strcmp(name,"meas_sigma_labor_w") == 0 ){ return x->meas_sigma_labor_w; }
 else if( strcmp(name,"meas_sigma_labor_m") == 0 ){ return x->meas_sigma_labor_m; }
 else if( strcmp(name,"meas_sigma_home_w") == 0 ){ return x->meas_sigma_home_w; }
 else if( strcmp(name,"meas_sigma_home_m") == 0 ){ return x->meas_sigma_home_m; }
 else if( strcmp(name,"meas_sigma_market") == 0 ){ return x->meas_sigma_market; }
 else if( strcmp(name,"meas_sigma_home_share_exp_w") == 0 ){ return x->meas_sigma_home_share_exp_w; }
 else {return NAN;}

}


int get_int_par_struct(par_struct* x, char* name){

 if( strcmp(name,"num_nodes") == 0 ){ return x->num_nodes; }
 else if( strcmp(name,"cpp_threads") == 0 ){ return x->cpp_threads; }
 else if( strcmp(name,"num_grid") == 0 ){ return x->num_grid; }
 else if( strcmp(name,"num_N") == 0 ){ return x->num_N; }
 else if( strcmp(name,"seed") == 0 ){ return x->seed; }
 else {return -9999;}

}


double* get_double_p_par_struct(par_struct* x, char* name){

 if( strcmp(name,"meas_sigma_vec") == 0 ){ return x->meas_sigma_vec; }
 else if( strcmp(name,"grid_labor") == 0 ){ return x->grid_labor; }
 else if( strcmp(name,"grid_home") == 0 ){ return x->grid_home; }
 else if( strcmp(name,"grid_market") == 0 ){ return x->grid_market; }
 else if( strcmp(name,"nodes") == 0 ){ return x->nodes; }
 else if( strcmp(name,"weights") == 0 ){ return x->weights; }
 else {return NULL;}

}


bool get_bool_par_struct(par_struct* x, char* name){

 if( strcmp(name,"reuse_init") == 0 ){ return x->reuse_init; }
 else if( strcmp(name,"do_gridsearch") == 0 ){ return x->do_gridsearch; }
 else if( strcmp(name,"do_meas_sigma_profile") == 0 ){ return x->do_meas_sigma_profile; }
 else {return false;}

}


