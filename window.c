/*!\file window.c
 * \author Farès BELHADJ, amsi@up8.edu
 * \réalisation Sérine TALEB
 * \date april 2025
 */

/* inclusion des entêtes de fonctions de création et de gestion de
 * fenêtres système ouvrant un contexte favorable à GL4dummies. Cette
 * partie est dépendante de la bibliothèque SDL2 */
#include <GL4D/gl4duw_SDL2.h>
#include <GL4D/gl4dm.h>
#include <GL4D/gl4dg.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "assimp.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "mobile.h"
#include "credit.h"

#define R_MASK 0x00FF0000
#define G_MASK 0x0000FF00
#define B_MASK 0x000000FF
#define A_MASK 0xFF000000

static void init(void);
static void draw(void);
static void simu(void);
static void quit(void);
static void resize(int w, int h);
static void initAudio(const char * file);
static void keyd(int keycode);
static void chargerTexture(GLuint id, const char * filename);
static void drawEtoiles(GLuint PID);
static void drawUneEtoile(GLuint PID, GLuint obj);
static void drawCredit();
static void vueHaut();


typedef struct cam_t cam_t;
struct cam_t {
  GLfloat x, y, z, a;
};

typedef struct {
  float x, y, z, z_mouv;
  float a;
  int active;
  int posFInale;
} cube_t;


#define NBR_CUBES 10
static cube_t _cubes[NBR_CUBES];
typedef struct { float x, z; } Position;
static Position _initialPos[NBR_CUBES]; //sauvegarder les positions de départ
static Mix_Music * _mmusic = NULL;
static cam_t _cam = {-0.1f, 2.0f, 6.168192f, M_PI / 2.0f};
static int _ww = 1800, _wh = 1700;

static GLuint _tex[8] = { 0, 0, 0 , 0, 0,0,0,0 };
static GLuint _pId = 0, _pIdB = 0;

static GLuint _cube = 0 ,  _plan = 0,_dome = 0, _disco = 0,_star = 0, _uneEtoile = 0;
static GLuint _vue = 0;
static const GLfloat _plan_scale = 300.0f;
static Uint32 _musicStartTime = 0;
static int _formerCercle = 0; // 0 = aucun cercle, 1 = formation du cercle 2 = fin formation
static Uint32 _startFormation = 0;
const float _rayon = 5.0f;
const float _dureeForm = 3.0f;
static float global_a = 0.0f;

 int lancerCredit = 0;
static int apparitionEtoile =  0, apparitionUneEtoile = 0;

/*!\brief créé la fenêtre, un screen 2D effacé en noir et lance une
 *  boucle infinie.*/
int main(int argc, char ** argv) {
  if(!gl4duwCreateWindow(argc, argv, "A few chords", 10, 10, _ww, _wh, GL4DW_SHOWN)) {
    return 1;
  }
  init();
  atexit(quit);
  gl4duwKeyDownFunc(keyd);
  gl4duwResizeFunc(resize);
  gl4duwIdleFunc(simu);
  gl4duwDisplayFunc(draw);
  gl4duwMainLoop();
  return 0;
}


static void init(void){
  glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
  SDL_GL_SetSwapInterval(1);
  glEnable(GL_DEPTH_TEST);
  _cube = gl4dgGenCubef();
  _plan = gl4dgGenQuadf();
  _dome = gl4dgGenSpheref(12, 12);
  _pId = gl4duCreateProgram("<vs>shaders/hello.vs", "<fs>shaders/hello.fs", NULL);
  _pIdB = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/basic.fs", NULL);

  gl4duGenMatrix(GL_FLOAT, "mod");
  gl4duGenMatrix(GL_FLOAT, "view");
  gl4duGenMatrix(GL_FLOAT, "proj");
  gl4duBindMatrix("proj");
  gl4duLoadIdentityf();

  GLfloat ratio = _wh / (GLfloat)_ww;
  
  /* combiner la matrice courante avec une matrice de projection en
     perspective. Voir le support de cours pour les six paramètres :
     left, right, bottom, top, near, far */
  gl4duFrustumf(-1.0f, 1.0f, -1.0f * ratio, 1.0f * ratio, 1.0f, _plan_scale);
  /* générer les identifiants de texture et les stocker dans _tex */
  glGenTextures(8, _tex);


  //charger les textures
  glBindTexture(GL_TEXTURE_2D, _tex[0]);
  chargerTexture(_tex[0], "images/espace.jpg");

 
  glBindTexture(GL_TEXTURE_2D, _tex[1]);
  chargerTexture(_tex[1], "images/Cracked_soil_Diffuse.jpg");

  glBindTexture(GL_TEXTURE_2D, _tex[2]);
  chargerTexture(_tex[2], "images/discoball.jpeg");

  glBindTexture(GL_TEXTURE_2D, _tex[3]);
  chargerTexture(_tex[3], "images/discoball_normal.jpeg");



  /*INIT LES CUBES */
  for(int i = 0; i < NBR_CUBES; ++i) {
  _cubes[i].x = i * 2.0f - (NBR_CUBES - 1); // pour les centrer
  _cubes[i].y = 0.5f;
  _cubes[i].z = -5.0f;
  _cubes[i].a = 0.0f;
  _cubes[i].active = 0;
}

  _disco = assimpGenScene("model/disco/scene.gltf");

  _star = assimpGenScene("model/star/scene.gltf");

  _uneEtoile = assimpGenScene("model/star/scene.gltf");

  /* on désactive la texture courante */
  glBindTexture(GL_TEXTURE_2D, 0);

  initAudio("music/a_few_chords.mod");
  mobile_init(180);
  init_credit();
}

static void vueHaut(){
    if(!_vue)  return;
    gl4duLookAtf(0.3f, 12.0f, -4.0f,
                0.3f, 6.0f, -4.0f, 
                0.0f, 0.0f, -1.0f);
  }

static void chargerTexture(GLuint id, const char * filename) {
  SDL_Surface * t;
  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  if( (t = IMG_Load(filename)) != NULL ) {
    int mode = t->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;      
    glTexImage2D(GL_TEXTURE_2D, 0, mode, t->w, t->h, 0, mode, GL_UNSIGNED_BYTE, t->pixels);
    SDL_FreeSurface(t);
  } else {
    fprintf(stderr, "can't open file %s : %s\n", filename, SDL_GetError());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  }
}

void initAudio(const char * file) {
  int mixFlags = MIX_INIT_OGG | MIX_INIT_MP3, res;
  res = Mix_Init(mixFlags);
  if( (res & mixFlags) != mixFlags ) {
    fprintf(stderr, "Mix_Init: Erreur lors de l'initialisation de la bibliotheque SDL_Mixer\n");
    fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
    exit(-3);
  }
  if(Mix_OpenAudio(44100, AUDIO_S16LSB, 2, 1024) < 0)
    exit(-4);
  if(!(_mmusic = Mix_LoadMUS(file))) {
    fprintf(stderr, "Erreur lors du Mix_LoadMUS: %s\n", Mix_GetError());
    exit(-5);
  }
  if(!Mix_PlayingMusic()){
    Mix_PlayMusic(_mmusic, -1);
    _musicStartTime = SDL_GetTicks();
  }
}


void draw(void){

   if (lancerCredit) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawCredit();
    return;
  }
  
  static float a = 0;
  gl4duBindMatrix("view");
  gl4duLoadIdentityf();

  if(_vue){
    vueHaut();
  }else{
  gl4duLookAtf(_cam.x, _cam.y, _cam.z, _cam.x + cos(_cam.a), _cam.y, _cam.z - sin(_cam.a), 0.0f, 1.0f, 0.0f);
}
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(_pId);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _tex[0]);
  glUniform1i(glGetUniformLocation(_pId, "tex"), 0);

  /* binder (mettre au premier plan, "en courante" ou "en active") la
     matrice mod */
  gl4duBindMatrix("mod");
  glUniform1f( glGetUniformLocation(_pId, "tex_scale"), 1.0f );
  glUniform2f( glGetUniformLocation(_pId, "tex_translate"), 0.0f, 0.0f );

  /*chargement texture cubes dans draw car problemes dans init*/
  glBindTexture(GL_TEXTURE_2D, _tex[4]);
  chargerTexture(_tex[4], "images/stars.jpg");


  /*DESSINS DES CUBES*/
  for (int i = 0; i < NBR_CUBES; ++i)
  {
    if(_cubes[i].active){
      glUniform4f(glGetUniformLocation(_pId, "couleur"), 1.0f, 1.0f, 1.0f, 1.0f );
      gl4duLoadIdentityf();
      gl4duPushMatrix();
      gl4duTranslatef(_cubes[i].x, _cubes[i].y, _cubes[i].z + _cubes[i].z_mouv);
      gl4duScalef(0.5f, 0.5f, 0.5f);
      gl4duRotatef(_cubes[i].a, 0.0f, 1.0f, 0.0f);
      gl4duSendMatrices();
      gl4duPopMatrix();
      gl4dgDraw(_cube);
    }
  }

 
  /* UN SOL */
  /* set le uniform couleur (dans le frag shader) à blanc */
  glUniform4f( glGetUniformLocation(_pId, "couleur"), 1.0f, 1.0f, 1.0f, 1.0f );
  gl4duLoadIdentityf();
  /* translation du sol vers le bas */
  gl4duTranslatef(_cam.x, -1.0f, _cam.z);
  /* rotation de -90° sur l'axe des x */
  gl4duRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
  gl4duScalef(_plan_scale, _plan_scale, _plan_scale);
  /* set le uniform tex_scale (dans le vertex shader) à la taille du scale */
  glUniform1f( glGetUniformLocation(_pId, "tex_scale"), _plan_scale );
  /* set le uniform tex_translate (dans le vertex shader) */
  glUniform2f( glGetUniformLocation(_pId, "tex_translate"), _cam.x / 2.0f, -_cam.z / 2.0f );
  gl4duSendMatrices();
  glBindTexture(GL_TEXTURE_2D, _tex[1]);
  gl4dgDraw(_plan);
  

  /* LE DOME */
  /* set le uniform couleur (dans le frag shader) à blanc */
  glUniform4f( glGetUniformLocation(_pId, "couleur"), 0.2f, 0.6f, 1.0f, 0.5f );
  gl4duLoadIdentityf();
  gl4duTranslatef(_cam.x, 0.0f, _cam.z);
  gl4duScalef(_plan_scale * 0.99f, _plan_scale * 0.99f, _plan_scale * 0.99f);
  /* set le uniform tex_scale (dans le vertex shader) à la taille du scale */
  glUniform1f( glGetUniformLocation(_pId, "tex_scale"), 5.0f );
  /* set le uniform tex_translate (dans le vertex shader) à zéro */
  glUniform2f( glGetUniformLocation(_pId, "tex_translate"), 0.0f, 0.0f );
  gl4duSendMatrices();
  glBindTexture(GL_TEXTURE_2D, _tex[0]);
  gl4dgDraw(_dome);
  
  /*BALLE DISCO*/
  gl4duBindMatrix("mod");
  gl4duLoadIdentityf();
  gl4duTranslatef(0.3f, 9.5f, -4.0f); 
  gl4duScalef(5.0f, 5.0f, 5.0f); // agrandissement boule
  gl4duRotatef(180.0f * a / M_PI, 0.0f, 1.0f, 0.0f);
  gl4duSendMatrices();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _tex[2]);
  glUniform1i(glGetUniformLocation(_pId, "use_tex"), 1);
  glUniform1i(glGetUniformLocation(_pId, "use_nm"), 0);
  float e = 0.5f + 0.5f * sin(SDL_GetTicks() * 0.005f); // varie entre 0 et 1
  glUniform3f(glGetUniformLocation(_pId, "emission"), e, e, e); // lumière blanche qui pulse
  glUniform1i(glGetUniformLocation(_pId, "is_disco"), 1);
  assimpDrawScene(_disco);
  glUniform1i(glGetUniformLocation(_pId, "is_disco"), 0); // désactiver disco


  drawEtoiles(_pIdB);
  drawUneEtoile(_pId, _uneEtoile);
  glUseProgram(0);
  a += 1;
}


/*UNE SEULE ETOILE*/
static void drawUneEtoile(GLuint PID, GLuint obj) {
    static float a = 0.0f;
    const float r = 90.0f;
    
    if(!apparitionUneEtoile) return;

    //calcul de l'a basé sur le temps
    a += r * gl4dGetElapsedTime() / 1000.0f;
    if(a > 360.0f) a -= 360.0f;

    gl4duBindMatrix("mod");
    gl4duLoadIdentityf();
    gl4duTranslatef(0.3f, 0.5f, -1.0f);
    gl4duScalef(1.0f, 1.0f, 1.0f);
    gl4duRotatef(a, 0.0f, 1.0f, 0.0f);
    gl4duSendMatrices();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _tex[6]);
    glUniform1i(glGetUniformLocation(PID, "use_tex"), 1);
    glUniform1i(glGetUniformLocation(PID, "use_nm"), 0);
    glUniform4f(glGetUniformLocation(PID, "color"), 1.0f, 0.96f, 0.0f, 0.8f);
    
    assimpDrawScene(obj);
}



/* PARTICULES ETOILES */
static void drawEtoiles(GLuint PID){
  if(!apparitionEtoile) return;   
  glUseProgram(PID);
  gl4duBindMatrix("mod");
  gl4duLoadIdentityf();
  glUniform4f(glGetUniformLocation(PID, "color"), 0.4f, 0.3f, 0.0f, 1.0f);
  mobile_draw(PID, _star);
  glUseProgram(0);
  /*apparitionEtoile = 0;

*/}

static void drawCredit(){
  if(!lancerCredit) return;

  draw_credit();
}

 

/* appelée lors du exit */
  static void quit(void) {
  for (int i = 0;i < 5;++i) {
    if(_tex[i]) {
      glDeleteTextures(sizeof _tex / sizeof *_tex, _tex);
      _tex[i] = 0;
    }
  }
  mobile_quit();
  sortie_credit();
  gl4duClean(GL4DU_ALL);
}


void simu(void) {
  gl4duBindMatrix("view");
  gl4duLoadIdentityf();

  mobile_simu();
    static double t0 = 0.0;
    double t = gl4dGetElapsedTime() / 1000.0, dt = t - t0;
    t0 = t;

    float musicPos = (SDL_GetTicks() - _musicStartTime) / 1000.0f;

    //apparition progressive cubes
    static double t_cubes = 0.0;
    const double delai = 3.9;
    static int cubeActuel = 0;

    if (t - t_cubes >= delai && cubeActuel < NBR_CUBES) {
        _cubes[cubeActuel].active = 1;
        cubeActuel++;
        t_cubes = t;
    }

    //comportement cubes
    for (int i = 0; i < NBR_CUBES; ++i) {
        if (_cubes[i].active) {
            if (_formerCercle < 2) {
                if (cubeActuel == NBR_CUBES) {

                    float neighbor_a = 0.0f;
                    if (i > 0) neighbor_a += _cubes[i - 1].a;
                    if (i < NBR_CUBES - 1) neighbor_a += _cubes[i + 1].a;

                    if (i % 2) _cubes[i].a += 0.02f + 0.005f * neighbor_a;

                    if (i % 2 != 0) {
                        _cubes[i].z_mouv = sin(SDL_GetTicks() * 0.005f + i) * 2.0f;
                    } else {
                        _cubes[i].a += 6.05f;
                    }
                } else {
                    _cubes[i].a += 3.05f;
                }

                if (musicPos >= 49.0f && i % 2 == 0) {
                    _cubes[i].z_mouv = -sin(SDL_GetTicks() * 0.005f + i) * 2.0f;
                }
            }
            _cubes[i].a += 0.5f * dt;
        }
    }

     if (musicPos >= 53.0f) _vue = 1;

     if (musicPos >= 57.0f) _vue = 0;
    //déclancher formation du cercle
    if (musicPos >= 60.0f && _formerCercle == 0) {

        _formerCercle = 1;
        _startFormation = SDL_GetTicks();

        for (int i = 0; i < NBR_CUBES; ++i) {
            if (_cubes[i].active) {
                _initialPos[i].x = _cubes[i].x;
                _initialPos[i].z = _cubes[i].z;
            }
        }
    }

    //animation de formation
    if (_formerCercle == 1) {

        float progression = (SDL_GetTicks() - _startFormation) / (_dureeForm * 1000.0f);
        progression = progression > 1.0f ? 1.0f : progression;

        for (int i = 0; i < NBR_CUBES; ++i) {
            if (_cubes[i].active) {
                float a = i * (2 * M_PI / NBR_CUBES);
                float target_x = _rayon * cos(a);
                float target_z = -_rayon * sin(a);

                _cubes[i].x = _initialPos[i].x + (target_x - _initialPos[i].x) * progression;
                _cubes[i].z = _initialPos[i].z + (target_z - _initialPos[i].z) * progression;

                if (progression >= 1.0f) {
                    _cubes[i].z_mouv = 0.0f;
                }
            }
        }

        if (progression >= 1.0f) {
            _formerCercle = 2;
        }
    }

    //mouvement circulaire des cubes
    if (_formerCercle == 2 ){
      apparitionUneEtoile = 1;
      if(musicPos >= 67.0f){
        apparitionEtoile = 1;
      }
        global_a += 7.5f * dt;
        for (int i = 0; i < NBR_CUBES; ++i) {
            if (_cubes[i].active){
                float a_cube = i * (2.0f * M_PI / NBR_CUBES);
                 float final_a = global_a + a_cube;
                _cubes[i].x = _rayon * cos(final_a);
                _cubes[i].z = -_rayon * sin(final_a);
            }
        }
    }

      if (musicPos >= 71.0f ) {
        lancerCredit = 1;
    }
    
}


/*!\brief Cette fonction paramétre la vue (viewport) OpenGL en
 * fonction des dimensions de la fenêtre.*/
static void resize(int w, int h) {
  _ww  = w; _wh = h;
  glViewport(0, 0, _ww, _wh);
  gl4duBindMatrix("projectionMatrix");
  gl4duLoadIdentityf();
  gl4duFrustumf(-0.5, 0.5, -0.5 * _wh / _ww, 0.5 * _wh / _ww, 1.0, 1000.0);
  gl4duBindMatrix("modelViewMatrix");
}

void keyd(int keycode) {
  switch(keycode) {
  case 'q':
    exit(0);
    break;
  default:
    break;
  }
}
