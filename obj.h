#ifndef OBJ_H
#define OBJ_H

/** ESTRUCTURAS **/

/****************************
 * Estructura para guardar  *
 * las coordenadas y las coordenadas de textura de puntos 3D *
 ****************************/
typedef struct punto
{
    float x, y, z, u, v;
} punto;

/****************************
 * Estructura para guardar  *
 * las coordenadas de puntos 3D *
 ****************************/
typedef struct
{
    double x, y, z;
} point3;

/*****************************
 * Estructura para guardar   *
 * las coordenadas de vectores 3D *
 *****************************/
typedef struct
{
    double x, y, z;
} vector3;

/****************************
 * Estructura para guardar  *
 * los colores en modo RGB  *
 ****************************/
typedef struct
{
    double r, g, b;
} color3;

/****************************
 * Estructura para guardar  *
 * la lista de matrices     *
 ****************************/

typedef struct mlist
{
    double m[16];
    struct mlist *hptr;
} mlist;

/***************************
 * Luz
 ***************************/

typedef struct light
{
    int onoff;
    int type; // 0 -> direccional, 1 -> posicional, 2 -> foco (spot light)
    color3 I;
    double pos[3]; // luz posicional o foco
    double campos[3];
    double dir[3]; // luz direccional o foco
    double camdir[3];
    double aperture; // cos(ang); si es 0 --> se ilumina cualquier posición.
                     //   si no es 0, solo se ilumina el cono.
} light;

/****************************
 * Estructura para guardar  *
 * los vértices de los objetos *
 ****************************/
typedef struct
{
    point3 coord; /* coordenadas, x, y, z */
    point3 camcoord;
    point3 proedcoord;
    double u, v;
    int num_faces; /* número de caras que comparten este vértice */
    double N[3];
    double Ncam[3];
    unsigned char rgb[3];
} vertex;

/****************************
 * Estructura para guardar  *
 * las caras o polígonos    *
 * de los objetos           *
 ****************************/
typedef struct
{
    int num_vertices;      /* número de vértices de la cara */
    int *vertex_ind_table; /* tabla con el índice de cada vértice */
    double N[3];
    double Ncam[3];
    unsigned char rgb[3];
} face;

/****************************
 * Estructura para guardar  *
 * una pila de objetos 3D   *
 ****************************/
struct object3d
{
    int num_vertices;     /* número de vértices del objeto */
    vertex *vertex_table; /* tabla de vértices */
    int num_faces;        /* número de caras del objeto */
    face *face_table;     /* tabla de caras */
    point3 min;           /* límites inferiores de las coordenadas */
    point3 max;           /* límites superiores de las coordenadas */
    mlist *mptr;
    color3 rgb;
    color3 ka;
    color3 kd;
    color3 ks;
    int ns;
    int texturaduna;
    struct object3d *hptr; /* siguiente elemento de la pila de objetos */
};

typedef struct object3d object3d;

int read_wavefront(char *file_name, object3d *object_ptr);

#endif /* OBJ_H */
