#include "common_object.h"


void compute_advection(pop *f, pop *rhs_f, my_double tau, pop *f_eq){

  int i,j,k,pp;
  my_double adv,aux;
#ifdef DEBUG
	char            fnamein[256], fnameout[256];
	char            name[256] = "NULL";
	FILE           *fin, *fout;
#endif

   my_double fac, dt_over_tau;
   pop ff_eq;
#ifdef METHOD_REDEFINED_POP
  my_double fac2;
  pop f0;
  pop fxp1, fxm1 ,fxm2 ,fxp2;
  pop fyp1, fym1, fym2, fyp2; 
  pop fzp1, fzm1 , fzm2, fzp2;
#endif

#ifdef DEBUG
	/* Each processor prints its mesh */
	sprintf(fnamein, "pop_p.%d.out", me);
	fout = fopen(fnamein, "w");
	
	for (k = 0; k < LNZ + TWO_BRD; k++)
	     for (j = 0; j < LNY + TWO_BRD; j++)
	       for (i = 0; i < LNX + TWO_BRD; i++){
		    //for(pp=0;pp<NPOP;pp++)
		    pp=7;
	fprintf(fout, "%d %d %d %d %e\n", i, j, k, pp, f[IDX(i,j,k)].p[pp] );
	       }
	fclose(fout);
#endif

#ifdef METHOD_REDEFINED_POP
      /* We store the equilibrium distribution in all points */
 for(k=BRD;k<LNZ+BRD;k++)
   for(j=BRD;j<LNY+BRD;j++)
    for(i=BRD;i<LNX+BRD;i++){ 
      	f_eq[IDX(i,j,k)]=equilibrium(f,i,j,k);
      }
  /* send the borders, needed to compute the advection */ 
     sendrecv_borders_pop(f_eq);

#ifdef LB_FLUID_BC
     boundary_conditions_for_equilibrium();
#endif
#endif

#ifdef METHOD_COLLISION_IMPLICIT
      /* We change f in  ( f + (dt/tau)f_eq ) /(1+dt/tau)  */
	dt_over_tau = property.time_dt/tau;  
	fac = 1./(1. + dt_over_tau);

/* We store the equilibrium distribution in all points */
 for(k=BRD;k<LNZ+BRD;k++)
   for(j=BRD;j<LNY+BRD;j++)
    for(i=BRD;i<LNX+BRD;i++){ 
      	f_eq[IDX(i,j,k)]=equilibrium(f,i,j,k);
      }

#ifdef LB_FLUID_BC
	/* bc for equilibrium function */
        boundary_conditions_for_equilibrium();
#endif
  for(k=BRD;k<LNZ+BRD;k++)
    for(j=BRD;j<LNY+BRD;j++)
      for(i=BRD;i<LNX+BRD;i++){ 
	for(pp=1;pp<NPOP;pp++) f[IDX(i,j,k)].p[pp] = fac*(f[IDX(i,j,k)].p[pp] + dt_over_tau* f_eq[IDX(i,j,k)].p[pp] );
      }
  /* send the borders, needed to compute the advection */ 
        sendrecv_borders_pop(f);
#endif

  /* check this index */
  for(k=BRD;k<LNZ+BRD;k++)
    for(j=BRD;j<LNY+BRD;j++)
      for(i=BRD;i<LNX+BRD;i++){ 

	/* population 0 does not advect anyhow*/
	rhs_f[IDX(i,j,k)].p[0] = 0.0;
	/* now taking care of pop 1 to 18 */
 	for(pp=1;pp<NPOP;pp++){

#ifdef METHOD_CENTERED	  
	  /* central difference scheme */
	  adv=0.0;
	  adv = compute_flux_with_central_difference(f,i,j,k,pp);
#endif

#ifdef METHOD_UPWIND
 /* first order upwind scheme */
	  adv=0.0;
	  adv = compute_flux_with_upwind_first(f,i,j,k,pp);	  
#endif

 /* QUICK */
#ifdef METHOD_MYQUICK
#ifndef METHOD_REDEFINED_POP

 /* quick scheme */
	  adv=0.0;
	  adv = compute_flux_with_quick(f,i,j,k,pp);

#else
 /* The population to be advected is different */
 /* it is  f + (dt/(2*tau))*(f_eq-f) */
 /* we prepare such a population here */
 fac = 0.5*(property.time_dt/tau); 


f0.p[pp]  = f[IDX(i,j,k)].p[pp] + fac*(f_eq[IDX(i,j,k)].p[pp] - f[IDX(i,j,k)].p[pp]);

fxp1.p[pp] = f[IDX(i+1,j,k)].p[pp] + fac*(f_eq[IDX(i+1,j,k)].p[pp] - f[IDX(i+1,j,k)].p[pp]);
fxm1.p[pp] = f[IDX(i-1,j,k)].p[pp] + fac*(f_eq[IDX(i-1,j,k)].p[pp] - f[IDX(i-1,j,k)].p[pp]);
fxm2.p[pp] = f[IDX(i-2,j,k)].p[pp] + fac*(f_eq[IDX(i-2,j,k)].p[pp] - f[IDX(i-2,j,k)].p[pp]);
fxp2.p[pp] = f[IDX(i+2,j,k)].p[pp] + fac*(f_eq[IDX(i+2,j,k)].p[pp] - f[IDX(i+2,j,k)].p[pp]);

fyp1.p[pp] = f[IDX(i,j+1,k)].p[pp] + fac*(f_eq[IDX(i,j+1,k)].p[pp] - f[IDX(i,j+1,k)].p[pp]);
fym1.p[pp] = f[IDX(i,j-1,k)].p[pp] + fac*(f_eq[IDX(i,j-1,k)].p[pp] - f[IDX(i,j-1,k)].p[pp]);
fym2.p[pp] = f[IDX(i,j-2,k)].p[pp] + fac*(f_eq[IDX(i,j-2,k)].p[pp] - f[IDX(i,j-2,k)].p[pp]);
fyp2.p[pp] = f[IDX(i,j+2,k)].p[pp] + fac*(f_eq[IDX(i,j+2,k)].p[pp] - f[IDX(i,j+2,k)].p[pp]);

fzp1.p[pp] = f[IDX(i,j,k+1)].p[pp] + fac*(f_eq[IDX(i,j,k+1)].p[pp] - f[IDX(i,j,k+1)].p[pp]);
fzm1.p[pp] = f[IDX(i,j,k-1)].p[pp] + fac*(f_eq[IDX(i,j,k-1)].p[pp] - f[IDX(i,j,k-1)].p[pp]);
fzm2.p[pp] = f[IDX(i,j,k-2)].p[pp] + fac*(f_eq[IDX(i,j,k-2)].p[pp] - f[IDX(i,j,k-2)].p[pp]);
fzp2.p[pp] = f[IDX(i,j,k+2)].p[pp] + fac*(f_eq[IDX(i,j,k+2)].p[pp] - f[IDX(i,j,k+2)].p[pp]);


 if(coeff_xp[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_xp[IDX(i,j,k)].p[pp] > 0.0){
   aux= 1.0 + interp2_xp[IDX(i,j,k)] - interp3_xm[IDX(i,j,k)];
   adv += coeff_xp[IDX(i,j,k)].p[pp]*( 
				      interp_xp[IDX(i,j,k)]*fxp1.p[pp] 
				      + (aux - interp_xp[IDX(i,j,k)])*f0.p[pp] 
				      - (aux + interp4_xm[IDX(i,j,k)])*fxm1.p[pp] 
				      + interp4_xm[IDX(i,j,k)]*fxm2.p[pp] 
				       );
   }else{
   aux = 1.0 + interp2_xm[IDX(i,j,k)] - interp3_xp[IDX(i,j,k)];
   adv += coeff_xp[IDX(i,j,k)].p[pp]*( 
				      (aux + interp4_xp[IDX(i,j,k)])*fxp1.p[pp] 
				      - interp4_xp[IDX(i,j,k)]*fxp2.p[pp] 
				      - interp_xm[IDX(i,j,k)]*fxm1.p[pp] 
				      - (aux - interp_xm[IDX(i,j,k)])*f0.p[pp] 
				       );
   }
 }


 if(coeff_yp[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_yp[IDX(i,j,k)].p[pp] > 0.0){
   aux = 1.0  + interp2_yp[IDX(i,j,k)] - interp3_ym[IDX(i,j,k)];
   adv += coeff_yp[IDX(i,j,k)].p[pp]*( 
				      interp_yp[IDX(i,j,k)]*fyp1.p[pp] 
				      + (aux - interp_yp[IDX(i,j,k)])*f0.p[pp] 
				      - (aux + interp4_ym[IDX(i,j,k)])*fym1.p[pp] 
				      + interp4_ym[IDX(i,j,k)]*fym2.p[pp] 
				       );
   }else{
   aux = 1.0 + interp2_ym[IDX(i,j,k)] - interp3_yp[IDX(i,j,k)];
   adv += coeff_yp[IDX(i,j,k)].p[pp]*( 
				      (aux + interp4_yp[IDX(i,j,k)])*fyp1.p[pp] 
				      - interp4_yp[IDX(i,j,k)]*fyp2.p[pp] 
				      - interp_ym[IDX(i,j,k)]*fym1.p[pp] 
				      - (aux - interp_ym[IDX(i,j,k)])*f0.p[pp] 
				       );
   }
 }


 if(coeff_zp[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_zp[IDX(i,j,k)].p[pp] > 0.0){
   aux = 1.0 + interp2_zp[IDX(i,j,k)] - interp3_zm[IDX(i,j,k)];
   adv += coeff_zp[IDX(i,j,k)].p[pp]*( 
				      interp_zp[IDX(i,j,k)]*fzp1.p[pp] 
				      + (aux - interp_zp[IDX(i,j,k)] )*f0.p[pp] 
				      - (aux + interp4_zm[IDX(i,j,k)])*fzm1.p[pp] 
				      + interp4_zm[IDX(i,j,k)]*fzm2.p[pp]
				       );
   }else{
   aux = 1.0 + interp2_zm[IDX(i,j,k)] - interp3_zp[IDX(i,j,k)];
   adv += coeff_zp[IDX(i,j,k)].p[pp]*( 
				      (aux + interp4_zp[IDX(i,j,k)])*fzp1.p[pp] 
				      - interp4_zp[IDX(i,j,k)]*fzp2.p[pp] 
				      -interp_zm[IDX(i,j,k)]*fzm1.p[pp] 
				      - (aux - interp_zm[IDX(i,j,k)] )*f0.p[pp] 
				       );
   }
 }

#endif /* end of redefined pop method */
#endif /* end of quick */

#ifdef METHOD_MYQUICK
#ifdef METHOD_MYQUICK_LIMITER
 adv=0.0;
 adv = compute_flux_with_limiters(f,i,j,k,pp);
 //fprintf(stderr,"adv %e\n",adv);
#endif
#endif

#ifdef METHOD_MIXED
adv=0.0;

if((LNY_END == NY && j==LNY+BRD-1  && c[pp].y > 0) || (LNY_START == 0 && j==BRD  && c[pp].y < 0)){

  adv = compute_flux_with_upwind_first(f,i,j,k,pp);
  //adv = compute_flux_with_limiters(f,i,j,k,pp);
	 }else{
#ifdef METHOD_CENTERED
        adv = compute_flux_with_central_difference(f,i,j,k,pp);
#endif
#ifdef METHOD_MYQUICK
        adv = compute_flux_with_quick(f,i,j,k,pp);
#endif
 }
#endif

/* with minus sign because we add it to the right hand side */
 rhs_f[IDX(i,j,k)].p[pp] = -adv;

 #ifdef DEBUG_HARD
 if(ROOT && itime ==1000) fprintf(stderr, " %d %d %d %d adv %e\n",i,j,k,pp,adv);
 #endif 

	}/* for pp */
      }/* for i, j , k */

#ifdef METHOD_STREAMING
#ifndef METHOD_STREAMING_INTERPOLATE
 for(k=BRD;k<LNZ+BRD;k++){
   for(j=BRD;j<LNY+BRD;j++){
      for(i=BRD;i<LNX+BRD;i++){ 

		rhs_f[IDX(i,j,k)]  = f[IDX(i,j,k)];
   
      }/* i */
   }/* j */
 }/* k */
#else
 /* here METHOD_STREAMING_INTERPOLATE is active */
 /* Interpolation method when the grid is cartesian rectangular, the quick algorithm can be written in a more compact form */ 
 if(coeff_xp[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_xp[IDX(i,j,k)].p[pp] > 0.0){
   aux= 1.0 + interp2_xp[IDX(i,j,k)] - interp3_xm[IDX(i,j,k)];
   adv += coeff_xp[IDX(i,j,k)].p[pp]*( 
				      interp_xp[IDX(i,j,k)]*f[IDX(i+1,j,k)].p[pp] 
				      + (aux - interp_xp[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] 
				      - (aux + interp4_xm[IDX(i,j,k)])*f[IDX(i-1,j,k)].p[pp] 
				      + interp4_xm[IDX(i,j,k)]*f[IDX(i-2,j,k)].p[pp] 
				       );
   }else{
   aux = 1.0 + interp2_xm[IDX(i,j,k)] - interp3_xp[IDX(i,j,k)];
   adv += coeff_xp[IDX(i,j,k)].p[pp]*( 
				      (aux + interp4_xp[IDX(i,j,k)])*f[IDX(i+1,j,k)].p[pp] 
				      - interp4_xp[IDX(i,j,k)]*f[IDX(i+2,j,k)].p[pp] 
				      - interp_xm[IDX(i,j,k)]*f[IDX(i-1,j,k)].p[pp] 
				      - (aux - interp_xm[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] 
				       );
   }
 }


 if(coeff_yp[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_yp[IDX(i,j,k)].p[pp] > 0.0){
   aux = 1.0  + interp2_yp[IDX(i,j,k)] - interp3_ym[IDX(i,j,k)];
   adv += coeff_yp[IDX(i,j,k)].p[pp]*( 
				      interp_yp[IDX(i,j,k)]*f[IDX(i,j+1,k)].p[pp] 
				      + (aux - interp_yp[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] 
				      - (aux + interp4_ym[IDX(i,j,k)])*f[IDX(i,j-1,k)].p[pp] 
				      + interp4_ym[IDX(i,j,k)]*f[IDX(i,j-2,k)].p[pp] 
				       );
   }else{
   aux = 1.0 + interp2_ym[IDX(i,j,k)] - interp3_yp[IDX(i,j,k)];
   adv += coeff_yp[IDX(i,j,k)].p[pp]*( 
				      (aux + interp4_yp[IDX(i,j,k)])*f[IDX(i,j+1,k)].p[pp] 
				      - interp4_yp[IDX(i,j,k)]*f[IDX(i,j+2,k)].p[pp] 
				      - interp_ym[IDX(i,j,k)]*f[IDX(i,j-1,k)].p[pp] 
				      - (aux - interp_ym[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] 
				       );
   }
 }


 if(coeff_zp[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_zp[IDX(i,j,k)].p[pp] > 0.0){
   aux = 1.0 + interp2_zp[IDX(i,j,k)] - interp3_zm[IDX(i,j,k)];
   adv += coeff_zp[IDX(i,j,k)].p[pp]*( 
				      interp_zp[IDX(i,j,k)]*f[IDX(i,j,k+1)].p[pp] 
				      + (aux - interp_zp[IDX(i,j,k)] )*f[IDX(i,j,k)].p[pp] 
				      - (aux + interp4_zm[IDX(i,j,k)])*f[IDX(i,j,k-1)].p[pp] 
				      + interp4_zm[IDX(i,j,k)]*f[IDX(i,j,k-2)].p[pp]
				       );
   }else{
   aux = 1.0 + interp2_zm[IDX(i,j,k)] - interp3_zp[IDX(i,j,k)];
   adv += coeff_zp[IDX(i,j,k)].p[pp]*( 
				      (aux + interp4_zp[IDX(i,j,k)])*f[IDX(i,j,k+1)].p[pp] 
				      - interp4_zp[IDX(i,j,k)]*f[IDX(i,j,k+2)].p[pp] 
				      -interp_zm[IDX(i,j,k)]*f[IDX(i,j,k-1)].p[pp] 
				      - (aux - interp_zm[IDX(i,j,k)] )*f[IDX(i,j,k)].p[pp] 
				       );
   }
 }
#endif
#endif
}/* end of compute_advection */

/* Advection methods */

#ifdef METHOD_CENTERED
my_double compute_flux_with_central_difference(pop * f, int i, int j, int k, int pp){

  my_double adv=0.0;

 /* centered difference scheme */
  
  /* for equispaced grid */
  /*	  
	  adv += coeff_xp[IDX(i,j,k)].p[pp]*0.5*(f[IDX(i+1,j,k)].p[pp] + f[IDX(i,j,k)].p[pp])+ 
	         coeff_xm[IDX(i,j,k)].p[pp]*0.5*(f[IDX(i-1,j,k)].p[pp] + f[IDX(i,j,k)].p[pp]);
	  
	  adv += coeff_yp[IDX(i,j,k)].p[pp]*0.5*(f[IDX(i,j+1,k)].p[pp] + f[IDX(i,j,k)].p[pp])+
	         coeff_ym[IDX(i,j,k)].p[pp]*0.5*(f[IDX(i,j-1,k)].p[pp] + f[IDX(i,j,k)].p[pp]);

          adv +=  coeff_zp[IDX(i,j,k)].p[pp]*0.5*(f[IDX(i,j,k+1)].p[pp] + f[IDX(i,j,k)].p[pp])+
                  coeff_zm[IDX(i,j,k)].p[pp]*0.5*(f[IDX(i,j,k-1)].p[pp] + f[IDX(i,j,k)].p[pp]);
	  */	  

  /* for stretched cartesian grid */	  	  
	  adv += coeff_xp[IDX(i,j,k)].p[pp]*( (1.0 - interp_xp[IDX(i,j,k)])* f[IDX(i+1,j,k)].p[pp] + interp_xp[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] )+ 
	         coeff_xm[IDX(i,j,k)].p[pp]*( (1.0 - interp_xm[IDX(i,j,k)])* f[IDX(i-1,j,k)].p[pp] + interp_xm[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] );
	  
	  adv += coeff_yp[IDX(i,j,k)].p[pp]*( (1.0 - interp_yp[IDX(i,j,k)])* f[IDX(i,j+1,k)].p[pp] + interp_yp[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] )+
	         coeff_ym[IDX(i,j,k)].p[pp]*( (1.0 - interp_ym[IDX(i,j,k)])* f[IDX(i,j-1,k)].p[pp] + interp_ym[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] );

          adv += coeff_zp[IDX(i,j,k)].p[pp]*( (1.0 - interp_zp[IDX(i,j,k)])* f[IDX(i,j,k+1)].p[pp] + interp_zp[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] )+
                 coeff_zm[IDX(i,j,k)].p[pp]*( (1.0 - interp_zm[IDX(i,j,k)])* f[IDX(i,j,k-1)].p[pp] + interp_zm[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] );

	  return adv;
}	 
#endif


#ifdef METHOD_UPWIND
my_double compute_flux_with_upwind_first(pop * f, int i, int j, int k, int pp){

  my_double adv=0.0;

 /* first order upwind scheme */
  
 if(coeff_xp[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_xp[IDX(i,j,k)].p[pp]*f[IDX(i,j,k)].p[pp];
 else
   adv += coeff_xp[IDX(i,j,k)].p[pp]*f[IDX(i+1,j,k)].p[pp];

 if(coeff_xm[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_xm[IDX(i,j,k)].p[pp]*f[IDX(i,j,k)].p[pp];
 else
   adv += coeff_xm[IDX(i,j,k)].p[pp]*f[IDX(i-1,j,k)].p[pp];

 if(coeff_yp[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_yp[IDX(i,j,k)].p[pp]*f[IDX(i,j,k)].p[pp];
 else
   adv += coeff_yp[IDX(i,j,k)].p[pp]*f[IDX(i,j+1,k)].p[pp];

 if(coeff_ym[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_ym[IDX(i,j,k)].p[pp]*f[IDX(i,j,k)].p[pp];
 else
   adv += coeff_ym[IDX(i,j,k)].p[pp]*f[IDX(i,j-1,k)].p[pp];

 if(coeff_zp[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_zp[IDX(i,j,k)].p[pp]*f[IDX(i,j,k)].p[pp];
 else
   adv += coeff_zp[IDX(i,j,k)].p[pp]*f[IDX(i,j,k+1)].p[pp];

 if(coeff_zm[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_zm[IDX(i,j,k)].p[pp]*f[IDX(i,j,k)].p[pp];
 else
   adv += coeff_zm[IDX(i,j,k)].p[pp]*f[IDX(i,j,k-1)].p[pp];


  /* second order upwind, to be checked ?*/
  
  /*
 if(coeff_xp[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_xp[IDX(i,j,k)].p[pp]*( (1.0 - interp_xp[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] + interp_xp[IDX(i,j,k)]*f[IDX(i-1,j,k)].p[pp] );
 else
   adv += coeff_xp[IDX(i,j,k)].p[pp]*( (1.0 - interp_xp[IDX(i,j,k)])*f[IDX(i+2,j,k)].p[pp] + interp_xp[IDX(i,j,k)]*f[IDX(i+1,j,k)].p[pp] );

 if(coeff_xm[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_xm[IDX(i,j,k)].p[pp]*( (1.0 - interp_xm[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] + interp_xm[IDX(i,j,k)]*f[IDX(i+1,j,k)].p[pp] );
 else
   adv += coeff_xm[IDX(i,j,k)].p[pp]*( (1.0 - interp_xm[IDX(i,j,k)])*f[IDX(i-2,j,k)].p[pp] + interp_xm[IDX(i,j,k)]*f[IDX(i-1,j,k)].p[pp] );

 if(coeff_yp[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_yp[IDX(i,j,k)].p[pp]*( (1.0 - interp_yp[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] + interp_yp[IDX(i,j,k)]*f[IDX(i,j-1,k)].p[pp] );
 else
   adv += coeff_yp[IDX(i,j,k)].p[pp]*( (1.0 - interp_yp[IDX(i,j,k)])*f[IDX(i,j+2,k)].p[pp] + interp_yp[IDX(i,j,k)]*f[IDX(i,j+1,k)].p[pp] );

 if(coeff_ym[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_ym[IDX(i,j,k)].p[pp]*( (1.0 - interp_ym[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] + interp_ym[IDX(i,j,k)]*f[IDX(i,j+1,k)].p[pp] );
 else
   adv += coeff_ym[IDX(i,j,k)].p[pp]*( (1.0 - interp_ym[IDX(i,j,k)])*f[IDX(i,j-2,k)].p[pp] + interp_ym[IDX(i,j,k)]*f[IDX(i,j-1,k)].p[pp] );

 if(coeff_zp[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_zp[IDX(i,j,k)].p[pp]*( (1.0 - interp_zp[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] + interp_zp[IDX(i,j,k)]*f[IDX(i,j,k-1)].p[pp] );
 else
   adv += coeff_zp[IDX(i,j,k)].p[pp]*( (1.0 - interp_zp[IDX(i,j,k)])*f[IDX(i,j,k+2)].p[pp] + interp_zp[IDX(i,j,k)]*f[IDX(i,j,k+1)].p[pp] );

 if(coeff_zm[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_zm[IDX(i,j,k)].p[pp]*( (1.0 - interp_zm[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] + interp_zm[IDX(i,j,k)]*f[IDX(i,j,k+1)].p[pp] );
 else
   adv += coeff_zm[IDX(i,j,k)].p[pp]*( (1.0 - interp_zm[IDX(i,j,k)])*f[IDX(i,j,k-2)].p[pp] + interp_zm[IDX(i,j,k)]*f[IDX(i,j,k-1)].p[pp] );
*/
 return adv;
}	  
#endif


#ifdef METHOD_MYQUICK
my_double compute_flux_with_quick(pop * f, int i, int j, int k, int pp){
 my_double aux;
 my_double adv=0.0;


/* QUICK for a regular grid */
/*
 my_double coeff_d, coeff_u, coeff_uu;
 coeff_d = 3.0/8.0;
 coeff_u = 6.0/8.0;
 coeff_uu = -1.0/8.0;
	  
 if(coeff_xp[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_xp[IDX(i,j,k)].p[pp]*( coeff_d*f[IDX(i+1,j,k)].p[pp] + coeff_u*f[IDX(i,j,k)].p[pp] + coeff_uu*f[IDX(i-1,j,k)].p[pp] );
 else
   adv += coeff_xp[IDX(i,j,k)].p[pp]*( coeff_d*f[IDX(i,j,k)].p[pp] + coeff_u*f[IDX(i+1,j,k)].p[pp] + coeff_uu*f[IDX(i+2,j,k)].p[pp] );

 if(coeff_xm[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_xm[IDX(i,j,k)].p[pp]*( coeff_d*f[IDX(i-1,j,k)].p[pp] + coeff_u*f[IDX(i,j,k)].p[pp] + coeff_uu*f[IDX(i+1,j,k)].p[pp] );
 else
   adv += coeff_xm[IDX(i,j,k)].p[pp]*( coeff_d*f[IDX(i,j,k)].p[pp] + coeff_u*f[IDX(i-1,j,k)].p[pp] + coeff_uu*f[IDX(i-2,j,k)].p[pp] );

 if(coeff_yp[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_yp[IDX(i,j,k)].p[pp]*( coeff_d*f[IDX(i,j+1,k)].p[pp] + coeff_u*f[IDX(i,j,k)].p[pp] + coeff_uu*f[IDX(i,j-1,k)].p[pp] );
 else
   adv += coeff_yp[IDX(i,j,k)].p[pp]*( coeff_d*f[IDX(i,j,k)].p[pp] + coeff_u*f[IDX(i,j+1,k)].p[pp] + coeff_uu*f[IDX(i,j+2,k)].p[pp] );

 if(coeff_ym[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_ym[IDX(i,j,k)].p[pp]*( coeff_d*f[IDX(i,j-1,k)].p[pp] + coeff_u*f[IDX(i,j,k)].p[pp] + coeff_uu*f[IDX(i,j+1,k)].p[pp] );
 else
   adv += coeff_ym[IDX(i,j,k)].p[pp]*( coeff_d*f[IDX(i,j,k)].p[pp] + coeff_u*f[IDX(i,j-1,k)].p[pp] + coeff_uu*f[IDX(i,j-2,k)].p[pp] );

 if(coeff_zp[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_zp[IDX(i,j,k)].p[pp]*( coeff_d*f[IDX(i,j,k+1)].p[pp] + coeff_u*f[IDX(i,j,k)].p[pp] + coeff_uu*f[IDX(i,j,k-1)].p[pp] );
 else
   adv += coeff_zp[IDX(i,j,k)].p[pp]*( coeff_d*f[IDX(i,j,k)].p[pp] + coeff_u*f[IDX(i,j,k+1)].p[pp] + coeff_uu*f[IDX(i,j,k+2)].p[pp] );

 if(coeff_zm[IDX(i,j,k)].p[pp] >= 0.0)
   adv += coeff_zm[IDX(i,j,k)].p[pp]*( coeff_d*f[IDX(i,j,k-1)].p[pp] + coeff_u*f[IDX(i,j,k)].p[pp] + coeff_uu*f[IDX(i,j,k+1)].p[pp] );
 else
   adv += coeff_zm[IDX(i,j,k)].p[pp]*( coeff_d*f[IDX(i,j,k)].p[pp] + coeff_u*f[IDX(i,j,k-1)].p[pp] + coeff_uu*f[IDX(i,j,k-2)].p[pp] );	  
 */

#ifdef METHOD_MYQUICK_CARTESIAN
 /* when the grid is cartesian rectangular, the quick algorithm can be written in a more compact form */ 
 //if(pp>0){
 if(coeff_xp[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_xp[IDX(i,j,k)].p[pp] > 0.0){
   aux= 1.0 + interp2_xp[IDX(i,j,k)] - interp3_xm[IDX(i,j,k)];
   adv += coeff_xp[IDX(i,j,k)].p[pp]*( 
				      interp_xp[IDX(i,j,k)]*f[IDX(i+1,j,k)].p[pp] 
				      + (aux - interp_xp[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] 
				      - (aux + interp4_xm[IDX(i,j,k)])*f[IDX(i-1,j,k)].p[pp] 
				      + interp4_xm[IDX(i,j,k)]*f[IDX(i-2,j,k)].p[pp] 
				       );
   }else{
   aux = 1.0 + interp2_xm[IDX(i,j,k)] - interp3_xp[IDX(i,j,k)];
   adv += coeff_xp[IDX(i,j,k)].p[pp]*( 
				      (aux + interp4_xp[IDX(i,j,k)])*f[IDX(i+1,j,k)].p[pp] 
				      - interp4_xp[IDX(i,j,k)]*f[IDX(i+2,j,k)].p[pp] 
				      - interp_xm[IDX(i,j,k)]*f[IDX(i-1,j,k)].p[pp] 
				      - (aux - interp_xm[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] 
				       );
   }
 }


 if(coeff_yp[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_yp[IDX(i,j,k)].p[pp] > 0.0){
   aux = 1.0  + interp2_yp[IDX(i,j,k)] - interp3_ym[IDX(i,j,k)];
   adv += coeff_yp[IDX(i,j,k)].p[pp]*( 
				      interp_yp[IDX(i,j,k)]*f[IDX(i,j+1,k)].p[pp] 
				      + (aux - interp_yp[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] 
				      - (aux + interp4_ym[IDX(i,j,k)])*f[IDX(i,j-1,k)].p[pp] 
				      + interp4_ym[IDX(i,j,k)]*f[IDX(i,j-2,k)].p[pp] 
				       );
   }else{
   aux = 1.0 + interp2_ym[IDX(i,j,k)] - interp3_yp[IDX(i,j,k)];
   adv += coeff_yp[IDX(i,j,k)].p[pp]*( 
				      (aux + interp4_yp[IDX(i,j,k)])*f[IDX(i,j+1,k)].p[pp] 
				      - interp4_yp[IDX(i,j,k)]*f[IDX(i,j+2,k)].p[pp] 
				      - interp_ym[IDX(i,j,k)]*f[IDX(i,j-1,k)].p[pp] 
				      - (aux - interp_ym[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] 
				       );
   }
 }


 if(coeff_zp[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_zp[IDX(i,j,k)].p[pp] > 0.0){
   aux = 1.0 + interp2_zp[IDX(i,j,k)] - interp3_zm[IDX(i,j,k)];
   adv += coeff_zp[IDX(i,j,k)].p[pp]*( 
				      interp_zp[IDX(i,j,k)]*f[IDX(i,j,k+1)].p[pp] 
				      + (aux - interp_zp[IDX(i,j,k)] )*f[IDX(i,j,k)].p[pp] 
				      - (aux + interp4_zm[IDX(i,j,k)])*f[IDX(i,j,k-1)].p[pp] 
				      + interp4_zm[IDX(i,j,k)]*f[IDX(i,j,k-2)].p[pp]
				       );
   }else{
   aux = 1.0 + interp2_zm[IDX(i,j,k)] - interp3_zp[IDX(i,j,k)];
   adv += coeff_zp[IDX(i,j,k)].p[pp]*( 
				      (aux + interp4_zp[IDX(i,j,k)])*f[IDX(i,j,k+1)].p[pp] 
				      - interp4_zp[IDX(i,j,k)]*f[IDX(i,j,k+2)].p[pp] 
				      -interp_zm[IDX(i,j,k)]*f[IDX(i,j,k-1)].p[pp] 
				      - (aux - interp_zm[IDX(i,j,k)] )*f[IDX(i,j,k)].p[pp] 
				       );
   }
 }
 //}/*end of if pp > 0 */
#else

 /* The good old one quick, less compact but well tested */
	  //if(pp>0){
 if(coeff_xp[IDX(i,j,k)].p[pp] != 0.0){
 if(coeff_xp[IDX(i,j,k)].p[pp] > 0.0)
   adv += coeff_xp[IDX(i,j,k)].p[pp]*( interp_xp[IDX(i,j,k)]*f[IDX(i+1,j,k)].p[pp] + (1.0 - interp_xp[IDX(i,j,k)] + interp2_xp[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] - interp2_xp[IDX(i,j,k)]*f[IDX(i-1,j,k)].p[pp] );
 else
   adv += coeff_xp[IDX(i,j,k)].p[pp]*( interp3_xp[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] + (1.0 - interp3_xp[IDX(i,j,k)] + interp4_xp[IDX(i,j,k)])*f[IDX(i+1,j,k)].p[pp] - interp4_xp[IDX(i,j,k)]*f[IDX(i+2,j,k)].p[pp] );
 }

 if(coeff_xm[IDX(i,j,k)].p[pp] != 0.0){
 if(coeff_xm[IDX(i,j,k)].p[pp] > 0.0)
   adv += coeff_xm[IDX(i,j,k)].p[pp]*( interp_xm[IDX(i,j,k)]*f[IDX(i-1,j,k)].p[pp] + (1.0 - interp_xm[IDX(i,j,k)] + interp2_xm[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] - interp2_xm[IDX(i,j,k)]*f[IDX(i+1,j,k)].p[pp] );
 else
   adv += coeff_xm[IDX(i,j,k)].p[pp]*( interp3_xm[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] + (1.0 - interp3_xm[IDX(i,j,k)] + interp4_xm[IDX(i,j,k)])*f[IDX(i-1,j,k)].p[pp] - interp4_xm[IDX(i,j,k)] *f[IDX(i-2,j,k)].p[pp] );
 }

 if(coeff_yp[IDX(i,j,k)].p[pp] != 0.0){
 if(coeff_yp[IDX(i,j,k)].p[pp] > 0.0)
   adv += coeff_yp[IDX(i,j,k)].p[pp]*( interp_yp[IDX(i,j,k)]*f[IDX(i,j+1,k)].p[pp] + (1.0 - interp_yp[IDX(i,j,k)] + interp2_yp[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] - interp2_yp[IDX(i,j,k)]*f[IDX(i,j-1,k)].p[pp] );
 else
   adv += coeff_yp[IDX(i,j,k)].p[pp]*( interp3_yp[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] + (1.0 - interp3_yp[IDX(i,j,k)] + interp4_yp[IDX(i,j,k)])*f[IDX(i,j+1,k)].p[pp] - interp4_yp[IDX(i,j,k)]*f[IDX(i,j+2,k)].p[pp] );
 }

 if(coeff_ym[IDX(i,j,k)].p[pp] != 0.0){
 if(coeff_ym[IDX(i,j,k)].p[pp] > 0.0)
   adv += coeff_ym[IDX(i,j,k)].p[pp]*( interp_ym[IDX(i,j,k)]*f[IDX(i,j-1,k)].p[pp] + (1.0 - interp_ym[IDX(i,j,k)] + interp2_ym[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] - interp2_ym[IDX(i,j,k)]*f[IDX(i,j+1,k)].p[pp] );
 else
   adv += coeff_ym[IDX(i,j,k)].p[pp]*( interp3_ym[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] + (1.0 - interp3_ym[IDX(i,j,k)] + interp4_ym[IDX(i,j,k)])*f[IDX(i,j-1,k)].p[pp] - interp4_ym[IDX(i,j,k)]*f[IDX(i,j-2,k)].p[pp] );
 }

 if(coeff_zp[IDX(i,j,k)].p[pp] != 0.0){
 if(coeff_zp[IDX(i,j,k)].p[pp] > 0.0)
   adv += coeff_zp[IDX(i,j,k)].p[pp]*( interp_zp[IDX(i,j,k)]*f[IDX(i,j,k+1)].p[pp] + (1.0 - interp_zp[IDX(i,j,k)] + interp2_zp[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] - interp2_zp[IDX(i,j,k)]*f[IDX(i,j,k-1)].p[pp] );
 else
   adv += coeff_zp[IDX(i,j,k)].p[pp]*( interp3_zp[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] + (1.0 - interp3_zp[IDX(i,j,k)] + interp4_zp[IDX(i,j,k)])*f[IDX(i,j,k+1)].p[pp] - interp4_zp[IDX(i,j,k)]*f[IDX(i,j,k+2)].p[pp] );
 }

 if(coeff_zm[IDX(i,j,k)].p[pp] != 0.0){
 if(coeff_zm[IDX(i,j,k)].p[pp] > 0.0)
   adv += coeff_zm[IDX(i,j,k)].p[pp]*( interp_zm[IDX(i,j,k)]*f[IDX(i,j,k-1)].p[pp] + (1.0 - interp_zm[IDX(i,j,k)] + interp2_zm[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] - interp2_zm[IDX(i,j,k)]*f[IDX(i,j,k+1)].p[pp] );
 else
   adv += coeff_zm[IDX(i,j,k)].p[pp]*( interp3_zm[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] + (1.0 - interp3_zm[IDX(i,j,k)] + interp4_zm[IDX(i,j,k)])*f[IDX(i,j,k-1)].p[pp] -interp4_zm[IDX(i,j,k)]*f[IDX(i,j,k-2)].p[pp] );	  
   }
 //}/*end of if pp > 0 */
#endif

 return adv;
}
#endif


/* flux limiters */
#ifdef METHOD_MYQUICK
#ifdef METHOD_MYQUICK_LIMITER
/* see http://en.wikipedia.org/wiki/Flux_limiter */
my_double limiter(my_double r1, my_double r2){

  my_double phi,b,r,r3;
  
  if(r2==0.0 || r1*r2 < 0.0){  
    r=0.0;
  }else{
    r=r1/r2;
  }  

  if(r1*r2<=0)phi=0;else phi=1.03;


  /* mine TVD */ 
  b=1.035; // poiseuille value 
  //b=1.14; // kolmogorov value
  /*
  if(r<0.5){ phi=2.*r; }else{
    if(r<1.0){ phi = 1.0; }else{
      if(r<b) phi=r; else phi=b;
    }
  }
  */
  /* osher */
  /*
  b=3.0;
  r3=MIN(r,b);
  phi=MAX(0,r3);
  */

  /* sweeby symmetric*/
  
  b=1.13; //poiseulle
  //b = 1.22; //kolmogorov
  /*
  r1=MIN(b*r,1.0);
  r2=MIN(r,b);
  r3=MAX(r1,r2); 
  phi=MAX(0,r3);
  */
  /* charm */
  //if(r>0) phi = r*(3.*r+1.)/pow((r+1.0),2.0); else phi=0.0;

  /* HQUICK */
  //phi=2.0*(r+fabs(r))/(r+3.0);

  /* smart */
  /*
  r1=2.*r;
  r2=(0.25+0.75*r);
  r3=MIN(r1,r2);
  r1=MIN(r3,4.0);
  phi=MAX(0.0,r1); 
  */

  /* superbee */
  /*
  r1 = MIN(2.0*r, 1.0);
  r2 = MIN(r, 2.0);
  r3 = MAX(r1,r2);
  phi = MAX(0.,r3);
  */

  /* minmod */
  /*  
  r1 = MIN(1,r);
  phi = MAX(0,r1);
  */

  /* van Leer */  
  //phi = (r+fabs(r))/(1.0+fabs(r));
  //phi = 2.*r/(r*r + 1.0);
  //phi = (r*r + r)/(r*r + 1.0);
  //phi=1.0;
  //fprintf(stderr,"r=%e\n",r);

  return phi;  
}


/************ Advection with flux limiter ****************/

my_double compute_flux_with_limiters(pop * f, int i, int j, int k, int pp){

  my_double advL,advH,adv,r,r1,r2;

  adv=0.0;

/* x */

 if(coeff_xp[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_xp[IDX(i,j,k)].p[pp] > 0.0){
     r1 =  (f[IDX(i,j,k)].p[pp] - f[IDX(i-1,j,k)].p[pp])/(center_V[IDX(i, j, k)].x - center_V[IDX(i-1, j, k)].x);
     r2 =  (f[IDX(i-1,j,k)].p[pp] - f[IDX(i-2,j,k)].p[pp])/(center_V[IDX(i-1, j, k)].x - center_V[IDX(i-2, j, k)].x);
   advL = coeff_xp[IDX(i,j,k)].p[pp]*f[IDX(i,j,k)].p[pp];
   advH = coeff_xp[IDX(i,j,k)].p[pp]*( interp_xp[IDX(i,j,k)]*f[IDX(i+1,j,k)].p[pp] + (1.0 - interp_xp[IDX(i,j,k)] + interp2_xp[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] - interp2_xp[IDX(i,j,k)]*f[IDX(i-1,j,k)].p[pp] );
   }else{
   r1 = (f[IDX(i,j,k)].p[pp] - f[IDX(i-1,j,k)].p[pp])/(center_V[IDX(i, j, k)].x - center_V[IDX(i-1, j, k)].x);
   r2 = ( f[IDX(i+1,j,k)].p[pp] - f[IDX(i,j,k)].p[pp])/(center_V[IDX(i+1, j, k)].x - center_V[IDX(i, j, k)].x);
   advL = coeff_xp[IDX(i,j,k)].p[pp]*f[IDX(i+1,j,k)].p[pp];
   advH = coeff_xp[IDX(i,j,k)].p[pp]*( interp3_xp[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] + (1.0 - interp3_xp[IDX(i,j,k)] + interp4_xp[IDX(i,j,k)])*f[IDX(i+1,j,k)].p[pp] - interp4_xp[IDX(i,j,k)]*f[IDX(i+2,j,k)].p[pp] );
   }
   adv += advL - limiter(r1,r2)*(advL - advH);
}



 if(coeff_xm[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_xm[IDX(i,j,k)].p[pp] > 0.0){
    r1 =  (f[IDX(i+1,j,k)].p[pp] - f[IDX(i,j,k)].p[pp] )/(center_V[IDX(i+1, j, k)].x - center_V[IDX(i, j, k)].x);
    r2 =  ( f[IDX(i+2,j,k)].p[pp] - f[IDX(i+1,j,k)].p[pp])/(center_V[IDX(i+2, j, k)].x - center_V[IDX(i+1, j, k)].x);
   advL = coeff_xm[IDX(i,j,k)].p[pp]*f[IDX(i,j,k)].p[pp];
   advH = coeff_xm[IDX(i,j,k)].p[pp]*( interp_xm[IDX(i,j,k)]*f[IDX(i-1,j,k)].p[pp] + (1.0 - interp_xm[IDX(i,j,k)] + interp2_xm[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] - interp2_xm[IDX(i,j,k)]*f[IDX(i+1,j,k)].p[pp] );
   }else{
   r1 =  (f[IDX(i+1,j,k)].p[pp] - f[IDX(i,j,k)].p[pp] )/(center_V[IDX(i+1, j, k)].x - center_V[IDX(i, j, k)].x);
   r2 =  (f[IDX(i,j,k)].p[pp] - f[IDX(i-1,j,k)].p[pp] )/(center_V[IDX(i, j, k)].x - center_V[IDX(i-1, j, k)].x);
   advL = coeff_xm[IDX(i,j,k)].p[pp]*f[IDX(i-1,j,k)].p[pp];
   advH = coeff_xm[IDX(i,j,k)].p[pp]*( interp3_xm[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] + (1.0 - interp3_xm[IDX(i,j,k)] + interp4_xm[IDX(i,j,k)])*f[IDX(i-1,j,k)].p[pp] - interp4_xm[IDX(i,j,k)] *f[IDX(i-2,j,k)].p[pp] );
   }
   adv += advL - limiter(r1,r2)*(advL - advH);
 }


/* y */

 if(coeff_yp[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_yp[IDX(i,j,k)].p[pp] > 0.0){
     r1 =  (f[IDX(i,j,k)].p[pp] - f[IDX(i,j-1,k)].p[pp])/(center_V[IDX(i, j, k)].y - center_V[IDX(i, j-1, k)].y);
     r2 =  (f[IDX(i,j-1,k)].p[pp] - f[IDX(i,j-2,k)].p[pp])/(center_V[IDX(i, j-1, k)].y - center_V[IDX(i, j-2, k)].y);
     advL = coeff_yp[IDX(i,j,k)].p[pp]*f[IDX(i,j,k)].p[pp];
   advH = coeff_yp[IDX(i,j,k)].p[pp]*( interp_yp[IDX(i,j,k)]*f[IDX(i,j+1,k)].p[pp] + (1.0 - interp_yp[IDX(i,j,k)] + interp2_yp[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] - interp2_yp[IDX(i,j,k)]*f[IDX(i,j-1,k)].p[pp] );
   }else{
    r1 =  (f[IDX(i,j,k)].p[pp] - f[IDX(i,j-1,k)].p[pp])/(center_V[IDX(i, j, k)].y - center_V[IDX(i, j-1, k)].y);
    r2 = ( f[IDX(i,j+1,k)].p[pp] - f[IDX(i,j,k)].p[pp])/(center_V[IDX(i, j+1, k)].y - center_V[IDX(i, j, k)].y);
  advL = coeff_yp[IDX(i,j,k)].p[pp]*f[IDX(i,j+1,k)].p[pp];
   advH = coeff_yp[IDX(i,j,k)].p[pp]*( interp3_yp[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] + (1.0 - interp3_yp[IDX(i,j,k)] + interp4_yp[IDX(i,j,k)])*f[IDX(i,j+1,k)].p[pp] - interp4_yp[IDX(i,j,k)]*f[IDX(i,j+2,k)].p[pp] );
   }
   adv += advL - limiter(r1,r2)*(advL - advH);
 }


 if(coeff_ym[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_ym[IDX(i,j,k)].p[pp] > 0.0){
    r1 =  (f[IDX(i,j+1,k)].p[pp] - f[IDX(i,j,k)].p[pp] )/(center_V[IDX(i, j+1, k)].y - center_V[IDX(i, j, k)].y);
    r2 =  ( f[IDX(i,j+2,k)].p[pp] - f[IDX(i,j+1,k)].p[pp])/(center_V[IDX(i, j+2, k)].y - center_V[IDX(i, j+1, k)].y);
   advL = coeff_ym[IDX(i,j,k)].p[pp]*f[IDX(i,j,k)].p[pp];
   advH = coeff_ym[IDX(i,j,k)].p[pp]*( interp_ym[IDX(i,j,k)]*f[IDX(i,j-1,k)].p[pp] + (1.0 - interp_ym[IDX(i,j,k)] + interp2_ym[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] - interp2_ym[IDX(i,j,k)]*f[IDX(i,j+1,k)].p[pp] );
   }else{
   r1 =  (f[IDX(i,j+1,k)].p[pp] - f[IDX(i,j,k)].p[pp] )/(center_V[IDX(i, j+1, k)].y - center_V[IDX(i, j, k)].y);
   r2 =  (f[IDX(i,j,k)].p[pp] - f[IDX(i,j-1,k)].p[pp] )/(center_V[IDX(i, j, k)].y - center_V[IDX(i, j-1, k)].y);
   advL = coeff_ym[IDX(i,j,k)].p[pp]*f[IDX(i,j-1,k)].p[pp];
   advH = coeff_ym[IDX(i,j,k)].p[pp]*( interp3_ym[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] + (1.0 - interp3_ym[IDX(i,j,k)] + interp4_ym[IDX(i,j,k)])*f[IDX(i,j-1,k)].p[pp] - interp4_ym[IDX(i,j,k)]*f[IDX(i,j-2,k)].p[pp] );
   }
   adv += advL - limiter(r1,r2)*(advL - advH);
 }



/* z */

 if(coeff_zp[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_zp[IDX(i,j,k)].p[pp] > 0.0){
    r1 =  (f[IDX(i,j,k)].p[pp] - f[IDX(i,j,k-1)].p[pp])/(center_V[IDX(i, j, k)].z - center_V[IDX(i, j, k-1)].z);
    r2 =   (f[IDX(i,j,k-1)].p[pp] - f[IDX(i,j,k-2)].p[pp])/(center_V[IDX(i, j, k-1)].z - center_V[IDX(i, j, k-2)].z);
   advL = coeff_zp[IDX(i,j,k)].p[pp]*f[IDX(i,j,k)].p[pp];
   advH = coeff_zp[IDX(i,j,k)].p[pp]*( interp_zp[IDX(i,j,k)]*f[IDX(i,j,k+1)].p[pp] + (1.0 - interp_zp[IDX(i,j,k)] + interp2_zp[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] - interp2_zp[IDX(i,j,k)]*f[IDX(i,j,k-1)].p[pp] );
   }else{
     r1 =  (f[IDX(i,j,k)].p[pp] - f[IDX(i,j,k-1)].p[pp] )/(center_V[IDX(i, j, k)].z - center_V[IDX(i, j, k-1)].z);
     r2 = ( f[IDX(i,j,k+1)].p[pp] - f[IDX(i,j,k)].p[pp])/(center_V[IDX(i, j, k+1)].z - center_V[IDX(i, j, k)].z);
   advL = coeff_zp[IDX(i,j,k)].p[pp]*f[IDX(i,j,k+1)].p[pp];
   advH = coeff_zp[IDX(i,j,k)].p[pp]*( interp3_zp[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] + (1.0 - interp3_zp[IDX(i,j,k)] + interp4_zp[IDX(i,j,k)])*f[IDX(i,j,k+1)].p[pp] - interp4_zp[IDX(i,j,k)]*f[IDX(i,j,k+2)].p[pp] );
   }
   adv += advL - limiter(r1,r2)*(advL - advH);
 }


 if(coeff_zm[IDX(i,j,k)].p[pp] != 0.0){
   if(coeff_zm[IDX(i,j,k)].p[pp] > 0.0){
     r1 =  (f[IDX(i,j,k+1)].p[pp] - f[IDX(i,j,k)].p[pp] )/(center_V[IDX(i, j, k+1)].z - center_V[IDX(i, j, k)].z);
     r2 =  (f[IDX(i,j,k+2)].p[pp] - f[IDX(i,j,k+1)].p[pp])/(center_V[IDX(i, j, k+2)].z - center_V[IDX(i, j, k+1)].z);
   advL = coeff_zm[IDX(i,j,k)].p[pp]*f[IDX(i,j,k)].p[pp];   
   advH = coeff_zm[IDX(i,j,k)].p[pp]*( interp_zm[IDX(i,j,k)]*f[IDX(i,j,k-1)].p[pp] + (1.0 - interp_zm[IDX(i,j,k)] + interp2_zm[IDX(i,j,k)])*f[IDX(i,j,k)].p[pp] - interp2_zm[IDX(i,j,k)]*f[IDX(i,j,k+1)].p[pp] );
   }else{
     r1 =  (f[IDX(i,j,k+1)].p[pp] - f[IDX(i,j,k)].p[pp] )/(center_V[IDX(i, j, k+1)].z - center_V[IDX(i, j, k)].z);
     r2 =  (f[IDX(i,j,k)].p[pp] - f[IDX(i,j,k-1)].p[pp] )/(center_V[IDX(i, j, k)].z - center_V[IDX(i, j, k-1)].z);
   advL = coeff_zm[IDX(i,j,k)].p[pp]*f[IDX(i,j,k-1)].p[pp];
   advH = coeff_zm[IDX(i,j,k)].p[pp]*( interp3_zm[IDX(i,j,k)]*f[IDX(i,j,k)].p[pp] + (1.0 - interp3_zm[IDX(i,j,k)] + interp4_zm[IDX(i,j,k)])*f[IDX(i,j,k-1)].p[pp] -interp4_zm[IDX(i,j,k)]*f[IDX(i,j,k-2)].p[pp] );
   }	  
   adv += advL - limiter(r1,r2)*(advL - advH);
 }


 return adv;
}
#endif
#endif


/*  Here Collision */

void add_collision(pop *f, pop *rhs_f, my_double tau,pop *f_eq){
  int i, j, k, pp;
  my_double invtau;
  pop ff_eq;
  pop f_eq_xp,f_eq_xm,f_eq_yp,f_eq_ym,f_eq_zp,f_eq_zm;
  pop fcoll, fcoll_xp,fcoll_xm,fcoll_yp,fcoll_ym,fcoll_zp,fcoll_zm;
  my_double fac;
 
    invtau = 1.0/tau;

#ifndef METHOD_COLLISION_IMPLICIT

  for(k=BRD;k<LNZ+BRD;k++)
    for(j=BRD;j<LNY+BRD;j++)
      for(i=BRD;i<LNX+BRD;i++){ 

#ifdef METHOD_REDEFINED_POP
	ff_eq = f_eq[IDX(i,j,k)];
#else     
	ff_eq=equilibrium(f,i,j,k);
#endif


#ifndef METHOD_EXPONENTIAL
       	for (pp=0; pp<NPOP; pp++) fcoll.p[pp] = -invtau * (f[IDX(i,j,k)].p[pp] - ff_eq.p[pp]);

#ifdef METHOD_LOG
	for (pp=0; pp<NPOP; pp++) fcoll.p[pp] =  ( exp(invtau*(ff_eq.p[pp] - f[IDX(i,j,k)].p[pp]) ) - 1.0);
#endif

#endif
     
#if (defined METHOD_MYQUICK && defined METHOD_TRAPEZOID)
	f_eq_xp = equilibrium(f,i+1,j,k); 
        f_eq_xm = equilibrium(f,i-1,j,k);
	f_eq_yp = equilibrium(f,i,j+1,k); 
        f_eq_ym = equilibrium(f,i,j-1,k);
	f_eq_zp = equilibrium(f,i,j,k+1); 
        f_eq_zm = equilibrium(f,i,j,k-1);

	for (pp=0; pp<NPOP; pp++){
	  fcoll_xp.p[pp] = -invtau * (f[IDX(i+1,j,k)].p[pp] - f_eq_xp.p[pp]);
	  fcoll_xm.p[pp] = -invtau * (f[IDX(i-1,j,k)].p[pp] - f_eq_xm.p[pp]);
	  fcoll_yp.p[pp] = -invtau * (f[IDX(i,j+1,k)].p[pp] - f_eq_yp.p[pp]);
	  fcoll_ym.p[pp] = -invtau * (f[IDX(i,j-1,k)].p[pp] - f_eq_ym.p[pp]);
	  fcoll_zp.p[pp] = -invtau * (f[IDX(i,j,k+1)].p[pp] - f_eq_zp.p[pp]);
	  fcoll_zm.p[pp] = -invtau * (f[IDX(i,j,k-1)].p[pp] - f_eq_zm.p[pp]);
	}
#endif
	
	for (pp=0; pp<NPOP; pp++){

	  //#define ONLY_COLLISION
#ifdef ONLY_COLLISION
	  /* set to zero , just for check to eliminate advection */
	  rhs_f[IDX(i,j,k)].p[pp] = 0.0;
#endif

	/* collision */
#ifdef METHOD_EXPONENTIAL
	  	rhs_f[IDX(i,j,k)].p[pp] +=   invtau * ff_eq.p[pp];
#else

		
#if (defined METHOD_MYQUICK && defined METHOD_TRAPEZOID)
   rhs_f[IDX(i,j,k)].p[pp] +=  0.25*fcoll.p[pp];

   rhs_f[IDX(i,j,k)].p[pp] += 0.125*( interp_xp[IDX(i,j,k)]*fcoll_xp.p[pp] + (1.0 - interp_xp[IDX(i,j,k)] + interp2_xp[IDX(i,j,k)])*fcoll.p[pp] - interp2_xp[IDX(i,j,k)]*fcoll_xm.p[pp] );
   rhs_f[IDX(i,j,k)].p[pp] += 0.125*( interp_xm[IDX(i,j,k)]*fcoll_xm.p[pp] + (1.0 - interp_xm[IDX(i,j,k)] + interp2_xm[IDX(i,j,k)])*fcoll.p[pp] - interp2_xm[IDX(i,j,k)]*fcoll_xp.p[pp] );

   rhs_f[IDX(i,j,k)].p[pp] += 0.125*( interp_yp[IDX(i,j,k)]*fcoll_yp.p[pp] + (1.0 - interp_yp[IDX(i,j,k)] + interp2_yp[IDX(i,j,k)])*fcoll.p[pp] - interp2_yp[IDX(i,j,k)]*fcoll_ym.p[pp] );
   rhs_f[IDX(i,j,k)].p[pp] += 0.125*( interp_ym[IDX(i,j,k)]*fcoll_ym.p[pp] + (1.0 - interp_ym[IDX(i,j,k)] + interp2_ym[IDX(i,j,k)])*fcoll.p[pp] - interp2_ym[IDX(i,j,k)]*fcoll_yp.p[pp] );

   rhs_f[IDX(i,j,k)].p[pp] += 0.125*( interp_zp[IDX(i,j,k)]*fcoll_zp.p[pp] + (1.0 - interp_zp[IDX(i,j,k)] + interp2_zp[IDX(i,j,k)])*fcoll.p[pp] - interp2_zp[IDX(i,j,k)]*fcoll_zm.p[pp] );
   rhs_f[IDX(i,j,k)].p[pp] += 0.125*( interp_zm[IDX(i,j,k)]*fcoll_zm.p[pp] + (1.0 - interp_zm[IDX(i,j,k)] + interp2_zm[IDX(i,j,k)])*fcoll.p[pp] - interp2_zm[IDX(i,j,k)]*fcoll_zp.p[pp] );

#else
   //rhs_f[IDX(i,j,k)].p[pp] +=  -invtau * (f[IDX(i,j,k)].p[pp] - f_eq.p[pp]);
			rhs_f[IDX(i,j,k)].p[pp] +=  fcoll.p[pp];
#endif
		
  //rhs_f[IDX(i,j,k)].p[pp] +=  -invtau * (f[IDX(i,j,k)].p[pp] - f_eq.p[pp]);
#endif

	}/* pp */
      }/* i,j,k */
#endif

}


#if (defined LB_FLUID_FORCING || defined LB_TEMPERATURE_FORCING)
void build_forcing(){
  int i, j, k;
  my_double fnx,fny,fnz,kn;
  my_double x,y,z;
  my_double LX,LY,LZ,nu;

   LX=(my_double)(property.SX);
   LY=(my_double)(property.SY);
   LZ=(my_double)(property.SZ);
   nu=property.nu;

 for(k=BRD;k<LNZ+BRD;k++)
    for(j=BRD;j<LNY+BRD;j++)
      for(i=BRD;i<LNX+BRD;i++){ 

#ifdef LB_FLUID_FORCING
	force[IDX(i,j,k)].x = 0.0;
	force[IDX(i,j,k)].y = 0.0;
	force[IDX(i,j,k)].z = 0.0;

#ifdef LB_FLUID_FORCING_POISEUILLE 
	/* note that the LX,LY,LZ dependence (i.e. the non-homogeneous direction) is here just an arbitrary choice */
	/* Here Amp indicates the maximal velocity at the center of the channel , note that in poiseuille flow U_max = Force * L^2/(8 \nu)  */
	force[IDX(i,j,k)].x += 2.0*property.Amp_x*((4.*nu)*pow(LY,-2.0));  
	force[IDX(i,j,k)].y += 2.0*property.Amp_y*((4.*nu)*pow(LX,-2.0));  
	force[IDX(i,j,k)].z += 2.0*property.Amp_z*((4.*nu)*pow(LY,-2.0));  
#endif

#ifdef LB_FLUID_FORCING_CHANNEL
	/* note that the LX,LY,LZ dependence (i.e. the non-homogeneous direction) is here just an arbitrary choice */
	/* Here Amp indicates the maximal velocity at the center of the turbulent channel , note that in this case we write U_max = sqrt(Force*L/2)  */
	force[IDX(i,j,k)].x += 2.0*pow(property.Amp_x,2.0)/LY;
	force[IDX(i,j,k)].y += 2.0*pow(property.Amp_y,2.0)/LX;
	force[IDX(i,j,k)].z += 2.0*pow(property.Amp_z,2.0)/LY;
#endif

#ifdef LB_FLUID_FORCING_CONSTANT_POWER
	/* Here Amp indicates the power Force*velocity  */
	/* note that out_all.uxt is computed only when we dumpe the averages */	
	if(out_all.ux != 0) force[IDX(i,j,k)].x += property.Amp_x/out_all.ux; 
	if(out_all.uy != 0) force[IDX(i,j,k)].y += property.Amp_y/out_all.uy;
	if(out_all.uz != 0) force[IDX(i,j,k)].z += property.Amp_z/out_all.uz;
#endif

#ifdef LB_FLUID_FORCING_KOLMOGOROV 
 
    kn=1.0;
    fnx=nu*pow(two_pi/LX,2.0);
    fny=nu*pow(two_pi/LY,2.0);
    y = (my_double)center_V[IDX(i,j,k)].y;
    x = (my_double)center_V[IDX(i,j,k)].x;

	/* along x */  
        force[IDX(i,j,k)].x += property.Amp_x*fny*sin(kn*two_pi*y/LY); 
	force[IDX(i,j,k)].y += property.Amp_y*fnx*sin(kn*two_pi*x/LX); 
	force[IDX(i,j,k)].z += property.Amp_z*fny*sin(kn*two_pi*y/LY);  

	//   fprintf(stderr,"property.Amp_x %e property.Amp_y %e property.Amp_z %e\n",property.Amp_x, property.Amp_y,property.Amp_z);
	//  exit(1);
#endif  


#ifdef LB_TEMPERATURE_BUOYANCY
  my_double temp, fac;

  //temp = (t[IDX(i,j,k)] - property.T_ref);
  temp =  t[IDX(i,j,k)] - 0.5*(property.T_bot + property.T_top);
  //temp =  t[IDX(i,j,k)] - (-(property.deltaT/property.SY)*center_V[IDX(i,j,k)].y + property.T_bot) ;
  fac = property.beta_t*temp; 
  if(property.beta2_t != 0.0) fac += property.beta2_t*temp*temp;
  //fac = property.beta_t;

      force[IDX(i,j,k)].x += fac*property.gravity_x;
      force[IDX(i,j,k)].y += fac*property.gravity_y;
      force[IDX(i,j,k)].z += fac*property.gravity_z;

      //fprintf(stderr, "fy %e\n",property.gravity_y);
#endif


#ifdef  LB_FLUID_FORCING_PENALIZATION
      my_double mask;
      /* penalization of a cube */
      /*  
      if( fabs(center_V[IDX(i,j,k)].x-property.SX/2.0) < 10 &&
	  fabs(center_V[IDX(i,j,k)].y) < 10 && 
	  fabs(center_V[IDX(i,j,k)].z-property.SZ/2.0) < 10  ) 
	mask=1.0; 
      else 
	mask=0.0;

      if( mask == 1.0 ){
	force[IDX(i,j,k)].x = -u[IDX(i,j,k)].x;  
	force[IDX(i,j,k)].y = -u[IDX(i,j,k)].y;
	force[IDX(i,j,k)].z = -u[IDX(i,j,k)].z;
	  }
      */
      /* small central spot penalization */
      mask = pow(center_V[IDX(i,j,k)].x-property.SX/2.0, 2.0)+pow(center_V[IDX(i,j,k)].y-property.SY/2.0, 2.0);
      if( mask < 1.0 ){       
	force[IDX(i,j,k)].x = -u[IDX(i,j,k)].x;  
	force[IDX(i,j,k)].y = -u[IDX(i,j,k)].y;
	force[IDX(i,j,k)].z = -u[IDX(i,j,k)].z;	
	  }
      
#endif


#endif

/* From here HERE SOURCE TERM ON SCALAR FIELD */
#ifdef LB_TEMPERATURE_FORCING
      /* set to zero */ 
      t_source[IDX(i,j,k)] = 0.0;

   /* here we can for instance impose a temperature profile , or add a thermal source or make the field reactive*/
 
#ifdef LB_TEMPERATURE_FORCING_SOURCE
  /* mimic source term  */
      my_double spot;
      /* penalization of a cube */
      /*        
      if( fabs(center_V[IDX(i,j,k)].x-property.SX/2.0) < 10 &&
	  fabs(center_V[IDX(i,j,k)].y) < 10 && 
	  fabs(center_V[IDX(i,j,k)].z-property.SZ/2.0) < 10  ) 
	spot=1.0; 
      else 
	spot=0.0;

      if( spot == 1.0 ) t_source[IDX(i,j,k)] = -(t[IDX(i,j,k)] - property.T_bot);
      */
      /* small central spot penalization */
      spot = pow(center_V[IDX(i,j,k)].x-property.SX/2.0, 2.0)+pow(center_V[IDX(i,j,k)].y-property.SY/2.0, 2.0);
      if( spot < 1.0 ) t_source[IDX(i,j,k)] = -(t[IDX(i,j,k)] - property.T_bot);
#endif

#ifdef LB_TEMPERATURE_FORCING_PROFILE
  /* impose a mean linear temperature profile , note that bc for temp shall be set to 0 */
  t_source[IDX(i,j,k)] = (property.deltaT/property.SY)*u[IDX(i,j,k)].y;
#endif

#ifdef LB_TEMPERATURE_FORCING_REACTION
  /* make the field reactive */
  t_source[IDX(i,j,k)] = t[IDX(i,j,k)]*(property.T_bot-t[IDX(i,j,k)]);
#endif

#ifdef LB_TEMPERATURE_FORCING_BULK
  t_source[IDX(i,j,k)] = property.Amp_t;
#ifdef LB_TEMPERATURE_FORCING_BULK_VARIABLE
  if (center_V[IDX(i,j,k)].x > property.SX/2.) t_source[IDX(i,j,k)] = 0.0;
#endif
#endif

#ifdef LB_TEMPERATURE_FORCING_RADIATION 
  my_double coeff_exct = 0.5/property.SY;
  t_source[IDX(i,j,k)] = property.Amp_t*coeff_exct*exp(-coeff_exct*center_V[IDX(i,j,k)].y); 
#endif  

#endif


      }/* i,j,k */
}
#endif

#if (defined LB_FLUID_FORCING || defined LB_TEMPERATURE_FORCING)
void add_forcing(){
  int i, j, k, pp;
  my_double invtau = 1.0/property.tau_u;
  pop f_eq;
  my_double fac;
  vector d;
  my_double ux,uy,uz,cu;
  vector vel;
  my_double rho ,temp;
  pop p_eq , g_eq;
  my_double mask;

#ifdef METHOD_FORCING_GUO
  fac = (1.0-0.5*invtau);
#endif

#ifdef METHOD_REDEFINED_POP
  fac = 1.0;//(1.0-property.time_dt*invtau);
#endif

  //#ifdef METHOD_COLLISION_IMPLICIT
  //fac = 1.0;// + exp(-property.time_dt*invtau);
  //#endif

  for(k=BRD;k<LNZ+BRD;k++)
    for(j=BRD;j<LNY+BRD;j++)
      for(i=BRD;i<LNX+BRD;i++){ 

	/* Here I prepare equilibrium pop for the direct forcing */
#ifdef  LB_FLUID_FORCING_DIRECT
      /* small central spot with velocity u=0 */
      mask = pow(center_V[IDX(i,j,k)].x-property.SX/2.0, 2.0)+pow(center_V[IDX(i,j,k)].y-property.SY/2.0, 2.0);
      vel.x = 0.0;
      vel.y = 0.0;
      vel.z = 0.0;
      rho = 1.0;
      p_eq = equilibrium_given_velocity(vel,rho);
#endif      

#ifdef  LB_TEMPERATURE_FORCING_DIRECT
      /* small central spot with velocity u=0 */
      mask = pow(center_V[IDX(i,j,k)].x-property.SX/2.0, 2.0)+pow(center_V[IDX(i,j,k)].y-property.SY/2.0, 2.0);
      vel.x = u[IDX(i,j,k)].x;
      vel.y = u[IDX(i,j,k)].y;
      vel.z = u[IDX(i,j,k)].z;
      temp = property.T_bot;
      g_eq = equilibrium_given_velocity(vel,temp);
#endif

      /* start loop on populations */
	for (pp=0; pp<NPOP; pp++){
	/* forcing */

#ifdef LB_FLUID_FORCING

#ifndef METHOD_FORCING_GUO	  	  	  
	  //#ifndef METHOD_REDEFINED_POP
          //  #ifndef METHOD_COLLISION_IMPLICIT
	    rhs_p[IDX(i,j,k)].p[pp] += 3.0*wgt[pp]*force[IDX(i,j,k)].x*c[pp].x;
            rhs_p[IDX(i,j,k)].p[pp] += 3.0*wgt[pp]*force[IDX(i,j,k)].y*c[pp].y;
            rhs_p[IDX(i,j,k)].p[pp] += 3.0*wgt[pp]*force[IDX(i,j,k)].z*c[pp].z;
	    //#else
	    ///* here METHOD_REDEFINED_POP is on */
	    //rhs_p[IDX(i,j,k)].p[pp] += fac*3.0*wgt[pp]*force[IDX(i,j,k)].x*c[pp].x;
            //rhs_p[IDX(i,j,k)].p[pp] += fac*3.0*wgt[pp]*force[IDX(i,j,k)].y*c[pp].y;
            //rhs_p[IDX(i,j,k)].p[pp] += fac*3.0*wgt[pp]*force[IDX(i,j,k)].z*c[pp].z;
	    //#endif
	    
#ifdef METHOD_LOG
	    fac = 3.0*wgt[pp]*property.tau_u*exp(-p[IDX(i,j,k)].p[pp]*invtau);
	    rhs_p[IDX(i,j,k)].p[pp] += fac*force[IDX(i,j,k)].x*c[pp].x;
            rhs_p[IDX(i,j,k)].p[pp] += fac*force[IDX(i,j,k)].y*c[pp].y;
            rhs_p[IDX(i,j,k)].p[pp] += fac*force[IDX(i,j,k)].z*c[pp].z;
#endif
	    
#else       
	ux=u[IDX(i,j,k)].x;
	uy=u[IDX(i,j,k)].y;
	uz=u[IDX(i,j,k)].z;
        cu = (c[pp].x*ux + c[pp].y*uy + c[pp].z*uz);
        d.x = (c[pp].x-ux)*invcs2 + c[pp].x*cu*invcs4;
        d.y = (c[pp].y-uy)*invcs2 + c[pp].y*cu*invcs4;
        d.z = (c[pp].z-uz)*invcs2 + c[pp].z*cu*invcs4;

       rhs_p[IDX(i,j,k)].p[pp] += fac*wgt[pp]*force[IDX(i,j,k)].x*d.x;
       rhs_p[IDX(i,j,k)].p[pp] += fac*wgt[pp]*force[IDX(i,j,k)].y*d.y;
       rhs_p[IDX(i,j,k)].p[pp] += fac*wgt[pp]*force[IDX(i,j,k)].z*d.z;       
#endif

#ifdef  LB_FLUID_FORCING_DIRECT

#ifdef LB_FLUID_FORCING_LANDSCAPE
       if(landscape[IDX(i, j, k)]>0.0){
#else
      if( sqrt(mask) < 10.0 ){
#endif
	/* this implementation works only when METHOD_EULER is used */
	rhs_p[IDX(i,j,k)].p[pp] =  (p_eq.p[pp] - p[IDX(i,j,k)].p[pp] )/property.time_dt;
	/* alterantively */
	//rhs_p[IDX(i,j,k)].p[pp] =  invtau*(p[IDX(i,j,k)].p[pp] -  p_eq.p[pp]);
	  }      
#endif

#endif

#ifdef LB_TEMPERATURE_FORCING
       /* Not Guo here ? */
	    rhs_g[IDX(i,j,k)].p[pp] += wgt[pp]*t_source[IDX(i,j,k)];
#endif

#ifdef  LB_TEMPERATURE_FORCING_DIRECT
      if( sqrt(mask) < 10.0 ){
	/* this implementation works only when METHOD_EULER is used */
	rhs_g[IDX(i,j,k)].p[pp] =  (p_eq.p[pp] - g[IDX(i,j,k)].p[pp] )/property.time_dt;
	/* alterantively */
	//rhs_g[IDX(i,j,k)].p[pp] =  invtau*(g[IDX(i,j,k)].p[pp] -  g_eq.p[pp]);
	  }      
#endif

	}/* pp */
      }/* i,j,k */
}
#endif


#ifdef LB_FLUID
/* be careful still not working */
tensor strain_tensor(pop *f,int i, int j, int k){
  int  pp;
  pop f_eq;
  tensor S;

  S.xx = S.xy = S.xz = 0.0;
  S.yx = S.yy = S.yz = 0.0;
  S.zx = S.zy = S.zz = 0.0;

      /* equilibrium distribution */
   f_eq=equilibrium(f,i,j,k);

      for (pp=0; pp<NPOP; pp++){
	S.xx += c[pp].x*c[pp].x*(f[IDX(i,j,k)].p[pp] - f_eq.p[pp]);
	S.yy += c[pp].y*c[pp].y*(f[IDX(i,j,k)].p[pp] - f_eq.p[pp]);
	S.zz += c[pp].z*c[pp].z*(f[IDX(i,j,k)].p[pp] - f_eq.p[pp]);
	S.xy += c[pp].x*c[pp].y*(f[IDX(i,j,k)].p[pp] - f_eq.p[pp]);
	S.xz += c[pp].x*c[pp].z*(f[IDX(i,j,k)].p[pp] - f_eq.p[pp]);
	S.yz += c[pp].y*c[pp].z*(f[IDX(i,j,k)].p[pp] - f_eq.p[pp]);
      }
      S.xy = S.yx;
      S.xz = S.zx;
      S.yz = S.zy;

      //fprintf(stderr,"SXY %e\n",f_eq.p[0]);
      return S;
}
#endif


#ifdef LB_FLUID
tensor gradient_vector(vector *t, int i, int j, int k){

  tensor tens;

/* in the bulk , centered 2nd order finite difference */
   tens.xx = ( t[IDX(i+1, j, k)].x - t[IDX(i-1, j, k)].x )/( center_V[IDX(i+1, j, k)].x - center_V[IDX(i-1, j, k)].x );
   tens.xy = ( t[IDX(i, j+1, k)].x - t[IDX(i, j-1, k)].x )/( center_V[IDX(i, j+1, k)].y - center_V[IDX(i, j-1, k)].y );
   tens.xz = ( t[IDX(i, j, k+1)].x - t[IDX(i, j, k-1)].x )/( center_V[IDX(i, j, k+1)].z - center_V[IDX(i, j, k-1)].z );

   tens.yx = ( t[IDX(i+1, j, k)].y - t[IDX(i-1, j, k)].y )/( center_V[IDX(i+1, j, k)].x - center_V[IDX(i-1, j, k)].x );
   tens.yy = ( t[IDX(i, j+1, k)].y - t[IDX(i, j-1, k)].y )/( center_V[IDX(i, j+1, k)].y - center_V[IDX(i, j-1, k)].y );
   tens.yz = ( t[IDX(i, j, k+1)].y - t[IDX(i, j, k-1)].y )/( center_V[IDX(i, j, k+1)].z - center_V[IDX(i, j, k-1)].z );

   tens.zx = ( t[IDX(i+1, j, k)].z - t[IDX(i-1, j, k)].z )/( center_V[IDX(i+1, j, k)].x - center_V[IDX(i-1, j, k)].x );
   tens.zy = ( t[IDX(i, j+1, k)].z - t[IDX(i, j-1, k)].z )/( center_V[IDX(i, j+1, k)].y - center_V[IDX(i, j-1, k)].y );
   tens.zz = ( t[IDX(i, j, k+1)].z - t[IDX(i, j, k-1)].z )/( center_V[IDX(i, j, k+1)].z - center_V[IDX(i, j, k-1)].z );

   /* at the x boundaries , one sided 1st order difference*/
if(LNX_START == 0 && i == BRD){
   tens.xx = ( t[IDX(i+1, j, k)].x - t[IDX(i, j, k)].x )/( center_V[IDX(i+1, j, k)].x - center_V[IDX(i, j, k)].x );
   tens.yx = ( t[IDX(i+1, j, k)].y - t[IDX(i, j, k)].y )/( center_V[IDX(i+1, j, k)].x - center_V[IDX(i, j, k)].x );
   tens.zx = ( t[IDX(i+1, j, k)].z - t[IDX(i, j, k)].z )/( center_V[IDX(i+1, j, k)].x - center_V[IDX(i, j, k)].x );
 }
 if(LNX_END == NY && i == LNX+BRD-1){ 
   tens.xx = ( t[IDX(i, j, k)].x - t[IDX(i-1, j, k)].x )/( center_V[IDX(i, j, k)].x - center_V[IDX(i-1, j, k)].x );
   tens.yx = ( t[IDX(i, j, k)].y - t[IDX(i-1, j, k)].y )/( center_V[IDX(i, j, k)].x - center_V[IDX(i-1, j, k)].x );
   tens.zx = ( t[IDX(i, j, k)].z - t[IDX(i-1, j, k)].z )/( center_V[IDX(i, j, k)].x - center_V[IDX(i-1, j, k)].x );
 }

   /* at the y boundaries */
if(LNY_START == 0 && j == BRD){
   tens.xy = ( t[IDX(i, j+1, k)].x - t[IDX(i, j, k)].x )/( center_V[IDX(i, j+1, k)].y - center_V[IDX(i, j, k)].y );
   tens.yy = ( t[IDX(i, j+1, k)].y - t[IDX(i, j, k)].y )/( center_V[IDX(i, j+1, k)].y - center_V[IDX(i, j, k)].y );
   tens.zy = ( t[IDX(i, j+1, k)].z - t[IDX(i, j, k)].z )/( center_V[IDX(i, j+1, k)].y - center_V[IDX(i, j, k)].y ); 
 }
 if(LNY_END == NY && j == LNY+BRD-1){ 
   tens.xy = ( t[IDX(i, j, k)].x - t[IDX(i, j-1, k)].x )/( center_V[IDX(i, j, k)].y - center_V[IDX(i, j-1, k)].y );
   tens.yy = ( t[IDX(i, j, k)].y - t[IDX(i, j-1, k)].y )/( center_V[IDX(i, j, k)].y - center_V[IDX(i, j-1, k)].y );
   tens.zy = ( t[IDX(i, j, k)].z - t[IDX(i, j-1, k)].z )/( center_V[IDX(i, j, k)].y - center_V[IDX(i, j-1, k)].y );
 }

   /* at the z boundaries */
if(LNZ_START == 0 && k == BRD){
   tens.xz = ( t[IDX(i, j, k+1)].x - t[IDX(i, j, k)].x )/( center_V[IDX(i, j, k+1)].z - center_V[IDX(i, j, k)].z );
   tens.yz = ( t[IDX(i, j, k+1)].y - t[IDX(i, j, k)].y )/( center_V[IDX(i, j, k+1)].z - center_V[IDX(i, j, k)].z );
   tens.zz = ( t[IDX(i, j, k+1)].z - t[IDX(i, j, k)].z )/( center_V[IDX(i, j, k+1)].z - center_V[IDX(i, j, k)].z );
 }
 if(LNZ_END == NZ && k == LNZ+BRD-1){ 
   tens.xz = ( t[IDX(i, j, k)].x - t[IDX(i, j, k-1)].x )/( center_V[IDX(i, j, k)].z - center_V[IDX(i, j, k-1)].z );
   tens.yz = ( t[IDX(i, j, k)].y - t[IDX(i, j, k-1)].y )/( center_V[IDX(i, j, k)].z - center_V[IDX(i, j, k-1)].z );
   tens.zz = ( t[IDX(i, j, k)].z - t[IDX(i, j, k-1)].z )/( center_V[IDX(i, j, k)].z - center_V[IDX(i, j, k-1)].z );
 }
      return tens;
}
#endif


#ifdef LB_TEMPERATURE
vector gradient_scalar(my_double *t, int i, int j, int k){

  vector grad;
  my_double a0,a1,a2,h1,h2;

  /* in the bulk , centered 2nd order finite difference */
   grad.x = ( t[IDX(i+1, j, k)] - t[IDX(i-1, j, k)] )/( center_V[IDX(i+1, j, k)].x - center_V[IDX(i-1, j, k)].x );
   grad.y = ( t[IDX(i, j+1, k)] - t[IDX(i, j-1, k)] )/( center_V[IDX(i, j+1, k)].y - center_V[IDX(i, j-1, k)].y );
   grad.z = ( t[IDX(i, j, k+1)] - t[IDX(i, j, k-1)] )/( center_V[IDX(i, j, k+1)].z - center_V[IDX(i, j, k-1)].z );


#define FIRST_ORDER_GRAD
#ifdef FIRST_ORDER_GRAD
   /* at the x boundaries , one sided 1st order difference*/
if(LNX_START == 0 && i == BRD){
    grad.x = ( t[IDX(i+1, j, k)] - t[IDX(i, j, k)] )/( center_V[IDX(i+1, j, k)].x - center_V[IDX(i, j, k)].x );
 }
 if(LNX_END == NY && i == LNX+BRD-1){ 
   grad.x = ( t[IDX(i, j, k)] - t[IDX(i-1, j, k)] )/( center_V[IDX(i, j, k)].x - center_V[IDX(i-1, j, k)].x );
 }

   /* at the y boundaries */
if(LNY_START == 0 && j == BRD){
   grad.y = ( t[IDX(i, j+1, k)] - t[IDX(i, j, k)] )/( center_V[IDX(i, j+1, k)].y - center_V[IDX(i, j, k)].y );
 }
 if(LNY_END == NY && j == LNY+BRD-1){ 
   grad.y = ( t[IDX(i, j, k)] - t[IDX(i, j-1, k)] )/( center_V[IDX(i, j, k)].y - center_V[IDX(i, j-1, k)].y );
 }

   /* at the z boundaries */
if(LNZ_START == 0 && k == BRD){
   grad.z = ( t[IDX(i, j, k+1)] - t[IDX(i, j, k)] )/( center_V[IDX(i, j, k+1)].z - center_V[IDX(i, j, k)].z ); 
 }
 if(LNZ_END == NZ && k == LNZ+BRD-1){ 
   grad.z = ( t[IDX(i, j, k)] - t[IDX(i, j, k-1)] )/( center_V[IDX(i, j, k)].z - center_V[IDX(i, j, k-1)].z );
 }
#endif


#ifdef SECOND_ORDER_GRAD
   /* at the x boundaries , one sided 2nd order difference*/
   /* but it seems to be less precise than the first order */
if(LNX_START == 0 && i == BRD){

  h1 =  center_V[IDX(i+1, j, k)].x - center_V[IDX(i, j, k)].x;
  h2 =  center_V[IDX(i+2, j, k)].x - center_V[IDX(i+1, j, k)].x; 
    a0 =  (2.0*h1+h2 )/( h1*(h1+h2) );
    a1 = -( h1+h2 )/( h1*h2 );
    a2 =  ( h1 )/( h2*(h1+h2) ); 
    grad.x = a0*t[IDX(i, j, k)] + a1*t[IDX(i+1, j, k)] + a2*t[IDX(i+2, j, k)];

 }
 if(LNX_END == NY && i == LNX+BRD-1){ 

   h1 =  center_V[IDX(i, j, k)].x - center_V[IDX(i-1, j, k)].x;
   h2 =  center_V[IDX(i-1, j, k)].x - center_V[IDX(i-2, j, k)].x; 
    a0 =  (2.0*h1+h2 )/( h1*(h1+h2) );
    a1 = -( h1+h2 )/( h1*h2 );
    a2 =  ( h1 )/( h2*(h1+h2) ); 
    grad.x = a0*t[IDX(i, j, k)] + a1*t[IDX(i-1, j, k)] + a2*t[IDX(i-2, j, k)];   
 }

   /* at the y boundaries */
if(LNY_START == 0 && j == BRD){
    h1 =  center_V[IDX(i, j+1, k)].y - center_V[IDX(i, j, k)].y;
    h2 =  center_V[IDX(i, j+2, k)].y - center_V[IDX(i, j+1, k)].y; 
    a0 =  (2.0*h1+h2 )/( h1*(h1+h2) );
    a1 = -( h1+h2 )/( h1*h2 );
    a2 =  ( h1 )/( h2*(h1+h2) ); 
    grad.y = a0*t[IDX(i, j, k)] + a1*t[IDX(i, j+1, k)] + a2*t[IDX(i, j+2, k)];
    //fprintf(stderr,"0 grad.y %e \n",grad.y);
 }
 if(LNY_END == NY && j == LNY+BRD-1){ 
    h1 =  center_V[IDX(i, j, k)].y - center_V[IDX(i, j-1, k)].y;
    h2 =  center_V[IDX(i, j-1, k)].y - center_V[IDX(i, j-2, k)].y; 
    a0 =  (2.0*h1+h2 )/( h1*(h1+h2) );
    a1 = -( h1+h2 )/( h1*h2 );
    a2 =  ( h1 )/( h2*(h1+h2) ); 
    grad.y = a0*t[IDX(i, j, k)] + a1*t[IDX(i, j-1, k)] + a2*t[IDX(i, j-2, k)]; 
    //fprintf(stderr,"50 grad.y %e \n",grad.y);
 }

   /* at the z boundaries */
if(LNZ_START == 0 && k == BRD){
  h1 =  center_V[IDX(i, j, k+1)].z - center_V[IDX(i, j, k)].z;
  h2 =  center_V[IDX(i, j, k+2)].z - center_V[IDX(i, j, k+1)].z; 
    a0 =  (2.0*h1+h2 )/( h1*(h1+h2) );
    a1 = -( h1+h2 )/( h1*h2 );
    a2 =  ( h1 )/( h2*(h1+h2) ); 
    grad.z = a0*t[IDX(i, j, k)] + a1*t[IDX(i, j, k+1)] + a2*t[IDX(i, j, k+2)];
 }
 if(LNZ_END == NZ && k == LNZ+BRD-1){ 
   h1 =  center_V[IDX(i, j, k)].z - center_V[IDX(i, j, k-1)].z;
   h2 =  center_V[IDX(i, j, k-1)].z - center_V[IDX(i, j, k-2)].z; 
    a0 =  (2.0*h1+h2 )/( h1*(h1+h2) );
    a1 = -( h1+h2 )/( h1*h2 );
    a2 =  ( h1 )/( h2*(h1+h2) ); 
    grad.z = a0*t[IDX(i, j, k)] + a1*t[IDX(i, j, k-1)] + a2*t[IDX(i, j, k-2)]; 
 }
#endif

    return grad;
}
#endif
