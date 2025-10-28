#include <GL4D/gl4duw_SDL2.h>
#include <GL4D/gl4dg.h>
#include <GL4D/gl4dp.h>
#include <GL4D/gl4dh.h>
#include <stdio.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "credit.h"

 void init_credit(void);
 void draw_credit(void);
 void sortie_credit(void);
 double get_dt(void);

static GLuint _quadId = 0;
static GLuint _pIdT = 0;
static GLuint _texId[1] = { 0 };
extern int lancerCredit;


void init_credit(void) {
  SDL_Surface * s = NULL, * d = NULL;
  TTF_Font * font = NULL;
  SDL_Color c = {131, 0, 252, 0};

  _quadId = gl4dgGenQuadf();
  _pIdT = gl4duCreateProgram("<vs>shaders/texte.vs", "<fs>shaders/texte.fs", NULL);

  /* initialiser la lib SDL2_ttf */
  if(TTF_Init() == -1) {
    fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
    exit(2);
  }
  /* chargement de la font */
  if( !(font = TTF_OpenFont("DejaVuSans-Bold.ttf", 120)) ) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    exit(2);
  }




  /* création d'une surface SDL avec le texte */
  s = TTF_RenderUTF8_Blended_Wrapped(font,
    "Réalisation: Sérine Taleb\nModèles 3D: -Star (https://skfb.ly/6WPtx)\n-Disco Ball (https://skfb.ly/oPGqG)\nTextures/Images :-free-3dtextureshd.com\n-freepik.com\nMusique: a_few_chords by paal granum (modarchive.org)"
    , c, 2048);
  if(s == NULL) {
    TTF_CloseFont(font);
    exit(2);
  }
  /* création d'une surface vierge et "compatible transfert" GL */
  d = SDL_CreateRGBSurface(0, s->w, s->h, 32, R_MASK, G_MASK, B_MASK, A_MASK);
  /* copie depuis s vers d */
  SDL_BlitSurface(s, NULL, d, NULL);
  /* libérer s */
  SDL_FreeSurface(s);

  /* transfert vers GL */
  glGenTextures(sizeof _texId / sizeof *_texId, _texId);
  assert(_texId[0] );

  glBindTexture(GL_TEXTURE_2D, _texId[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, d->w, d->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, d->pixels);

  glBindTexture(GL_TEXTURE_2D, 0);
  /* libérer d */
  SDL_FreeSurface(d);
  /* libérer la font */
  TTF_CloseFont(font);


  gl4duGenMatrix(GL_FLOAT, "mod");
  gl4duGenMatrix(GL_FLOAT, "view");
  gl4duGenMatrix(GL_FLOAT, "proj_id");
  gl4duBindMatrix("proj_id");
  gl4duLoadIdentityf();

}

double get_dt(void){
  static double t0 = 0.0f;

  double t = gl4dGetElapsedTime(), dt = (t - t0) / 1000.0;
  t0 = t;
  return dt;
}

void draw_credit(void) {
    static GLfloat position = -5.0f;
    static GLfloat t0 = -1;
    GLfloat t;

    if (lancerCredit == 1){
        if (t0 < 0.0f)
            t0 = SDL_GetTicks();
        
        t = (SDL_GetTicks() - t0) / 1000.0f;

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(_pIdT);

        gl4duBindMatrix("view");
        gl4duLoadIdentityf();
        gl4duBindMatrix("mod");
        gl4duLoadIdentityf();

        //déplacement vertical du texte
        position += 0.061f * get_dt();

        // Appliquer la translation
        gl4duTranslatef(0.0f, position, 0.0f); 
        gl4duSendMatrices(); 

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _texId[0]);
        glUniform1i(glGetUniformLocation(_pIdT, "use_tex"), 1);
        glUniform4f(glGetUniformLocation(_pIdT, "couleur"), 0.0f, 0.0f, 0.0f, 0.0f);
        gl4dgDraw(_quadId);

        glUseProgram(0);

        //quitter la fenêtre
        if (t >= 15.0f){  
          lancerCredit = 0;
          exit(0);
        }
    }
}

void sortie_credit(void) {
  if(_texId[0]) {
    glDeleteTextures(sizeof _texId / sizeof *_texId, _texId);
    _texId[0] = 0;
  }
}

