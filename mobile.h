/*!\file mobile.h
 *
 * \brief 
 * \author Farès BELHADJ, amsi@up8.edu
 * \date March 27, 2025
 */

#ifndef _MOBILE_H

#define _MOBILE_H

#include <GL4D/gl4du.h>

#ifdef __cplusplus
extern "C" {
#endif

  extern void mobile_init(int n);
  extern void mobile_simu(void);
  extern void mobile_draw(GLuint pId, GLuint modele);
  extern void mobile_quit(void);
  
#ifdef __cplusplus
}
#endif

#endif

