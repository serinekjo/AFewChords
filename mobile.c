#include "mobile.h"
#include "assimp.h"

typedef struct vec3d_t vec3d_t;
typedef struct mobile_t mobile_t;

struct vec3d_t {
  GLfloat x, y, z;
};

struct mobile_t {
  vec3d_t p, v;
  GLfloat r, amp_zigzag, freq_zigzag;
  GLfloat color[4];
  int nb_collisions;
};

/* gravité */
static const GLfloat _ig = 9.81f / 800.0f;
static vec3d_t _g = {0.0f, _ig, 0.0f};
static const GLfloat e = 2.5f / 9.0f;

/* tous les mobiles de ma scène */
static mobile_t * _mobiles = NULL;
static int _nb_mobiles = 0;

static inline void _reinit(int i) {
  _mobiles[i].r = 0.1f + 0.3f * gl4dmURand();
  _mobiles[i].p.x = 0.3f;
  _mobiles[i].p.y = -1.0f + _mobiles[i].r;
  _mobiles[i].p.z = -6.0f;
  _mobiles[i].v.x = 0.01f * gl4dmSURand();
  _mobiles[i].v.y = 9.5f + 2.0f * gl4dmURand();
  _mobiles[i].v.z = 0.01f * gl4dmSURand();

  if (gl4dmURand() < 0.5f) {
    _mobiles[i].color[0] = 1.0f;
    _mobiles[i].color[1] = 1.0f;
    _mobiles[i].color[2] = 1.0f;
  } else {
    _mobiles[i].color[0] = 1.0f;
    _mobiles[i].color[1] = 1.0f;
    _mobiles[i].color[2] = 0.0f;
  }
  _mobiles[i].color[3] = 1.0f;

  _mobiles[i].amp_zigzag = 0.0003f * gl4dmSURand();
  _mobiles[i].freq_zigzag = 10.0f * gl4dmSURand();
  _mobiles[i].nb_collisions = 0;
}

/*explosion particules*/
static inline void _reinit2(int i) {
  _mobiles[i].p.x = -1.5f + 3.0f * gl4dmURand();
  _mobiles[i].p.y = -1.5f + 3.0f * gl4dmURand(); 
  _mobiles[i].p.z = -4.0f + 0.5f * gl4dmURand();
  
  _mobiles[i].v.x = 0.0f;
  _mobiles[i].v.y = 0.0f;
  _mobiles[i].v.z = 5.0f + 1.0f * gl4dmURand(); // aller vers caméra (z=0)
  
  _mobiles[i].r = 0.2f + 0.2f * gl4dmURand(); // petit au début

  if (gl4dmURand() < 0.5f) {
    _mobiles[i].color[0] = 1.0f;
    _mobiles[i].color[1] = 1.0f;
    _mobiles[i].color[2] = 1.0f;
  } else {
    _mobiles[i].color[0] = 1.0f;
    _mobiles[i].color[1] = 1.0f;
    _mobiles[i].color[2] = 0.0f;
  }
  _mobiles[i].color[3] = 1.0f;

  _mobiles[i].amp_zigzag = 0.0f;
  _mobiles[i].freq_zigzag = 0.0f;
  _mobiles[i].nb_collisions = 0;
}



void mobile_init(int n) {
  assert(_mobiles == NULL);
  _nb_mobiles = n;
  _mobiles = malloc(_nb_mobiles * sizeof *_mobiles);
  assert(_mobiles);
  for(int i = 0; i < _nb_mobiles; ++i) {
    _reinit(i);
  }
}
void mobile_simu(void) {
  static double t0 = 0;
  double t = gl4dGetElapsedTime() / 1000.0, dt = t - t0;
  t0 = t;

  static int mode_transition = 0;

  if(t >= 70.0f && !mode_transition) {
    for(int i = 0; i < _nb_mobiles; ++i) {
      _reinit2(i);
    }
    mode_transition = 1;
  }

  for(int i = 0; i < _nb_mobiles; ++i) {
    if (!mode_transition) {
      int collision = 0;
      _mobiles[i].p.x += _mobiles[i].v.x * dt;
      _mobiles[i].p.y += _mobiles[i].v.y * dt;
      _mobiles[i].p.z += _mobiles[i].v.z * dt;

      if(_mobiles[i].p.y - _mobiles[i].r <= -1.0f) {
        if(_mobiles[i].v.y < 0.0f)
          _mobiles[i].v.y = -_mobiles[i].v.y;
        collision = 1;
      }
      if(_mobiles[i].p.y >= 3.0f) {
        _reinit(i);
      }

      if(collision) {
        _mobiles[i].v.x *= e;
        _mobiles[i].v.y *= e;
        _mobiles[i].v.z *= e;
      }
      _mobiles[i].v.x += _g.x * dt;
      _mobiles[i].v.x += _mobiles[i].amp_zigzag * sin(_mobiles[i].freq_zigzag * t);
      _mobiles[i].v.y += _g.y * dt;
      _mobiles[i].v.z += _g.z * dt;
    } else {
      if(_mobiles[i].p.z > -1.0f) {
        _mobiles[i].v.x = 0.0f;
        _mobiles[i].v.y = 0.0f;
        _mobiles[i].v.z = 0.0f;
      } else {
       
        _mobiles[i].p.z += _mobiles[i].v.z * dt;
      }

      //fait grossir les particules pour remplir l'écran
      _mobiles[i].r += 4.0f * dt;
      if(_mobiles[i].r > 10.0f) _mobiles[i].r = 10.0f;
    }
  }
}


void mobile_draw(GLuint pId, GLuint modele) {
  for(int i = 0; i < _nb_mobiles; ++i) {
    gl4duPushMatrix();
    gl4duTranslatef(_mobiles[i].p.x, _mobiles[i].p.y, _mobiles[i].p.z);
    gl4duScalef(_mobiles[i].r, _mobiles[i].r, _mobiles[i].r);
    glUniform4fv(glGetUniformLocation(pId, "color"), 1, _mobiles[i].color);
    assimpDrawScene(modele);
    gl4duPopMatrix();
  }
}

void mobile_quit(void) {
  if(_mobiles) {
    free(_mobiles);
    _mobiles = NULL;
    _nb_mobiles = 0;
  }
}
