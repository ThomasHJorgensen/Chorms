typedef struct sim_struct
{
 double* wage_w;
 double* wage_m;
 bool* work_flex_w;
 bool* work_flex_m;
 double* norm_w;
 double* norm_m;
 double* norm_alt_w;
 double* norm_alt_m;
 long long* nkids;
 double* labor_w;
 double* labor_m;
 double* home_w;
 double* home_m;
 double* market;
 double* home_share_exp_w;
} sim_struct;

double* get_double_p_sim_struct(sim_struct* x, char* name){

 if( strcmp(name,"wage_w") == 0 ){ return x->wage_w; }
 else if( strcmp(name,"wage_m") == 0 ){ return x->wage_m; }
 else if( strcmp(name,"norm_w") == 0 ){ return x->norm_w; }
 else if( strcmp(name,"norm_m") == 0 ){ return x->norm_m; }
 else if( strcmp(name,"norm_alt_w") == 0 ){ return x->norm_alt_w; }
 else if( strcmp(name,"norm_alt_m") == 0 ){ return x->norm_alt_m; }
 else if( strcmp(name,"labor_w") == 0 ){ return x->labor_w; }
 else if( strcmp(name,"labor_m") == 0 ){ return x->labor_m; }
 else if( strcmp(name,"home_w") == 0 ){ return x->home_w; }
 else if( strcmp(name,"home_m") == 0 ){ return x->home_m; }
 else if( strcmp(name,"market") == 0 ){ return x->market; }
 else if( strcmp(name,"home_share_exp_w") == 0 ){ return x->home_share_exp_w; }
 else {return NULL;}

}


bool* get_bool_p_sim_struct(sim_struct* x, char* name){

 if( strcmp(name,"work_flex_w") == 0 ){ return x->work_flex_w; }
 else if( strcmp(name,"work_flex_m") == 0 ){ return x->work_flex_m; }
 else {return NULL;}

}


long long* get_long_long_p_sim_struct(sim_struct* x, char* name){

 if( strcmp(name,"nkids") == 0 ){ return x->nkids; }
 else {return NULL;}

}


