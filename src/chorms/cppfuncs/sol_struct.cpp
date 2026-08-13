typedef struct sol_struct
{
 double* labor_w;
 double* labor_m;
 double* home_w;
 double* home_m;
 double* market;
 double* home_share_exp_w;
 double* init_labor_w;
 double* init_labor_m;
 double* init_home_w;
 double* init_home_m;
 double* init_market;
} sol_struct;

double* get_double_p_sol_struct(sol_struct* x, char* name){

 if( strcmp(name,"labor_w") == 0 ){ return x->labor_w; }
 else if( strcmp(name,"labor_m") == 0 ){ return x->labor_m; }
 else if( strcmp(name,"home_w") == 0 ){ return x->home_w; }
 else if( strcmp(name,"home_m") == 0 ){ return x->home_m; }
 else if( strcmp(name,"market") == 0 ){ return x->market; }
 else if( strcmp(name,"home_share_exp_w") == 0 ){ return x->home_share_exp_w; }
 else if( strcmp(name,"init_labor_w") == 0 ){ return x->init_labor_w; }
 else if( strcmp(name,"init_labor_m") == 0 ){ return x->init_labor_m; }
 else if( strcmp(name,"init_home_w") == 0 ){ return x->init_home_w; }
 else if( strcmp(name,"init_home_m") == 0 ){ return x->init_home_m; }
 else if( strcmp(name,"init_market") == 0 ){ return x->init_market; }
 else {return NULL;}

}


