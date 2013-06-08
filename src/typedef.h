#include "define.h"

//#define my_double long double 
#define my_double double

/* Useful structures */
typedef struct {
  my_double x;
  my_double y;
  my_double z;
} vector;

/* Useful structures */
typedef struct {
  my_double xx,xy,xz;
  my_double yx,yy,yz;
  my_double zx,zy,zz;
} tensor;

typedef struct {
  my_double p[19];
} pop;

typedef struct {
  my_double val;
  my_double name;
} param;

typedef struct {
  my_double NX , NY , NZ;
  my_double SX , SY , SZ; 
  my_double time_dt, time_start, time_end, time_dump_field, time_dump_diagn;
#ifdef LB_FLUID
  my_double tau_u , nu;
#ifdef LB_FLUID_FORCING
  my_double Amp_x,Amp_y,Amp_z;
#endif
#ifdef LB_TEMPERATURE
  my_double tau_t, kappa;
  my_double T_bot,T_top,T_ref,deltaT;
#ifdef LB_TEMPERATURE_BUOYANCY
  my_double beta_t,beta2_t;
  my_double gravity_x,gravity_y,gravity_z;
#endif
#ifdef LB_TEMPERATURE_MELTING
  my_double T_solid,latent_heat,specific_heat;
#endif
#endif
#ifdef LB_SCALAR
  my_double tau_s, chi;
#endif
#endif
} prop;


/* useful macro */
#define SIZE_STRUCT(x) (sizeof(x) / sizeof(my_double))

#define NPROP SIZE_STRUCT(prop)


typedef struct {
#ifdef LB_FLUID
  my_double x,y,z;
  my_double ux,uy,uz;
  my_double ux2,uy2,uz2;
  my_double rho,ene,eps;
#endif
#ifdef LB_TEMPERATURE
  my_double t,t2,epst;
  my_double dxt,dyt,dzt;
  my_double uxt,uyt,uzt;
  my_double nux,nuy,nuz;
#ifdef LB_TEMPERATURE_MELTING	
  my_double lf, dtlf, enth;
#endif		
#endif
	
} output;

#define NOUT SIZE_STRUCT(output)


/* WARNING mvx means rho*vx momentum ,  m means rho density*/
#define mvx(a) (a.p[1] -a.p[2] +a.p[7]  +a.p[8]  -a.p[9]  -a.p[10] +a.p[11] -a.p[12]+a.p[13]-a.p[14])
#define mvy(a) (a.p[3] -a.p[4] +a.p[7]  -a.p[8]  +a.p[9]  -a.p[10] +a.p[15] +a.p[16]-a.p[17]-a.p[18])
#define mvz(a) (a.p[5] -a.p[6] +a.p[11] +a.p[12] -a.p[13] -a.p[14] +a.p[15] -a.p[16]+a.p[17]-a.p[18])
#define m(a) (a.p[0]+a.p[1]+a.p[2]+a.p[3]+a.p[4]+a.p[5]+a.p[6]+a.p[7]+a.p[8]+a.p[9]+a.p[10]+a.p[11]+a.p[12]+a.p[13]+a.p[14]+a.p[15]+a.p[16]+a.p[17]+a.p[18])

#define ROOT (!me)

/* total frame size */
#ifdef METHOD_MYQUICK
#define BRD 2
#else 
#define BRD 1
#endif
#define TWO_BRD 2*BRD

/* index on the grid */
#define IDXG(i,j,k) ( (int)(k)*(LNYG+TWO_BRD)*(LNXG+TWO_BRD)+(int)(j)*(LNXG+TWO_BRD)+(int)(i) )
/* and index for grid planes */
#define IDXG_XBRD(i,j,k) ( (int)(k)*(LNYG+TWO_BRD)*(BRD)+(int)(j)*(BRD)+(int)(i) )
#define IDXG_YBRD(i,j,k) ( (int)(k)*(BRD)*(LNXG+TWO_BRD)+(int)(j)*(LNXG+TWO_BRD)+(int)(i) )
#define IDXG_ZBRD(i,j,k) ( (int)(k)*(LNYG+TWO_BRD)*(LNXG+TWO_BRD)+(int)(j)*(LNXG+TWO_BRD)+(int)(i) )


/* index on the vertices */
#define IDX(i,j,k) ( (int)(k)*(LNY+TWO_BRD)*(LNX+TWO_BRD)+(int)(j)*(LNX+TWO_BRD)+(int)(i) )
/* and index for vertices planes for BRD thickness slice */
#define IDX_XBRD(i,j,k) ( (int)(k)*(LNY+TWO_BRD)*(BRD)+(int)(j)*(BRD)+(int)(i) )
#define IDX_YBRD(i,j,k) ( (int)(k)*(BRD)*(LNX+TWO_BRD)+(int)(j)*(LNX+TWO_BRD)+(int)(i) )
#define IDX_ZBRD(i,j,k) ( (int)(k)*(LNY+TWO_BRD)*(LNX+TWO_BRD)+(int)(j)*(LNX+TWO_BRD)+(int)(i) )

/* and index for vertices planes for unit thickness slice*/
#define IDX_X(j,k) ( (int)(k)*(LNY+TWO_BRD)+(int)(j))
#define IDX_Y(i,k) ( (int)(k)*(LNX+TWO_BRD)+(int)(i))
#define IDX_Z(i,j) ( (int)(j)*(LNX+TWO_BRD)+(int)(i))

/* index for corner cubelets */
#define IDX_CORNER(i,j,k) ( (int)(k)*BRD*BRD+(int)(j)*BRD+(int)(i) )

/* neighbouring index for processors*/
#define IDX_NEXT(i,j,k) ( (int)(k+1)*9+(int)(j+1)*3+(int)(i+1) )


#define two_pi 2*3.14159265359
