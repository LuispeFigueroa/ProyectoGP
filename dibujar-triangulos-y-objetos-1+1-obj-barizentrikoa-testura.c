//	Programa desarrollado por
//
//	Facultad de Informática
//	Universidad del País Vasco
//	http://www.ehu.eus/if
//
// para compilarlo: gcc dibujar-triangulos-y-objetos.c cargar-ppm.c load_obj_joseba.c -lGL -lGLU -lglut -lm
//
//  o simplemente:    gcc *.c -lGL -lGLU -lglut -lm
//

#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
// #include "cargar-triangulo.h"
#include "obj.h"

// información de textura

extern int load_ppm(char *file, unsigned char **bufferptr, int *dimxptr, int *dimyptr);
unsigned char *bufferra;
int dimx, dimy, dimentsioa;

int indexx;
// hiruki *triangulosptr;
object3d *foptr;
object3d *sel_ptr;
int denak;
int lineak;
int objektuak;
int kamera;
char aldaketa;
int ald_lokala;
int objektuaren_ikuspegia;
int normalak_marraztu;
int flatmode;
int paralelo;
char fitxiz[100];
int atzearpegiakmarraztu;
mlist *camera_ptr;

// camara/observador
double MCSR[16];
double view_half_width;
double view_half_height;
double view_near;
double view_far;
double projection_matrix[16];

// TODO  m = m1 * m2
// Multiplicar m1 por m2 y guardar el resultado en la matriz m.
void mxm(double *m, double *m1, double *m2)
{
    int i, j, k;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        {
            m[i * 4 + j] = 0.0;
            for (k = 0; k < 4; k++)
                m[i * 4 + j] += m1[i * 4 + k] * m2[k * 4 + j];
        }
}

// tranformacion globar a la camara M_nueva = T x M_cam
void kamerari_aldaketa_sartu_ezk(double *m)
{
    mlist *new_matrix;

    new_matrix = (mlist *)malloc(sizeof(mlist));
    mxm(new_matrix->m, m, camera_ptr->m);
    new_matrix->hptr = camera_ptr;
    camera_ptr = new_matrix;
}

// aplicar transformacion local a la camara: M_nueva = M_cam · T
void kamerari_aldaketa_sartu_esk(double m[16])
{
    mlist *new_matrix;

    new_matrix = (mlist *)malloc(sizeof(mlist));
    mxm(new_matrix->m, camera_ptr->m, m);
    new_matrix->hptr = camera_ptr;
    camera_ptr = new_matrix;
}

// aplicar transformaciones al objeto
void objektuari_aldaketa_sartu_ezk(double m[16])
{
    mlist *new_matrix;

    new_matrix = (mlist *)malloc(sizeof(mlist));
    mxm(new_matrix->m, m, sel_ptr->mptr->m);
    new_matrix->hptr = sel_ptr->mptr;
    sel_ptr->mptr = new_matrix;
}

// aplicar transformacion loca
void objektuari_aldaketa_sartu_esk(double m[16])
{
    mlist *new_matrix;

    new_matrix = (mlist *)malloc(sizeof(mlist));
    mxm(new_matrix->m, sel_ptr->mptr->m, m);
    new_matrix->hptr = sel_ptr->mptr;
    sel_ptr->mptr = new_matrix;
}

// dado u,v obtener el puntero al color
unsigned char *color_textura(float u, float v)
{
    int indx;
    int indy;

    // coordenadas texturas dentro de 0-1
    if (u < 0.0f)
        u = 0.0f;
    else if (u > 1.0f)
        u = 1.0f;

    if (v < 0.0f)
        v = 0.0f;
    else if (v > 1.0f)
        v = 1.0f;

    // convertir coordenadas de textura a coordenadas pixel
    indx = (int)(u * (float)(dimx - 1));
    indy = (int)((1.0f - v) * (float)(dimy - 1));

    return bufferra + 3 * (indy * dimx + indx);
}

void print_matrizea16(double *m)
{
    int i;

    for (i = 0; i < 4; i++)
        printf("%lf, %lf, %lf, %lf\n", m[i * 4], m[i * 4 + 1], m[i * 4 + 2], m[i * 4 + 3]);
}

// TODO  res = m * v
// Multiplicar el vector v por la matriz m y guardar el resultado en el vector señalado por res.
// Se supone que el cuarto componente del vector v y del resultado res es 0.
void mxv(double *res, double *m, double *v)
{
    res[0] = v[0];
    res[1] = v[1];
    res[2] = v[2];
}

// TODO  pptr = m * p
// Multiplicar el punto p por la matriz m y guardar el resultado en el punto señalado por pptr.
// Se supone que el cuarto componente del punto p es 1.
// Si según la cuarta fila de la matriz el cuarto componente del resultado, w, no es 1, entonces hay que devolver su equivalente: x/w, y/w y z/w
void mxp(point3 *pptr, double m[16], point3 p)
{
    double x, y, z, w;

    // Matrix 4x4; p = (x, y, z, 1)
    x = m[0] * p.x + m[1] * p.y + m[2] * p.z + m[3];
    y = m[4] * p.x + m[5] * p.y + m[6] * p.z + m[7];
    z = m[8] * p.x + m[9] * p.y + m[10] * p.z + m[11];
    w = m[12] * p.x + m[13] * p.y + m[14] * p.z + m[15];

    // si w no es 1
    if (w != 1.0 && w != 0.0)
    {
        x /= w;
        y /= w;
        z /= w;
    }

    pptr->x = x;
    pptr->y = y;
    pptr->z = z;
}

// TODO obtener las coordenadas que tienen los vértices y los vectores normales del objeto en el sistema de referencia de la cámara
// MCSR = matriz de transformación del sistema de coordenadas del objeto al sistema de coordenadas de la cámara
void get_mcsr(double *M)
{
    double *A = camera_ptr->m;
    double Xc[3], Yc[3], Zc[3], E[3];

    // ejes y posicion de la camara
    Xc[0] = A[0];
    Xc[1] = A[4];
    Xc[2] = A[8];

    Yc[0] = A[1];
    Yc[1] = A[5];
    Yc[2] = A[9];

    Zc[0] = A[2];
    Zc[1] = A[6];
    Zc[2] = A[10];

    E[0] = A[3];
    E[1] = A[7];
    E[2] = A[11];

    // filas de la matriz mcsr, ejes de la camara
    M[0] = Xc[0];
    M[1] = Xc[1];
    M[2] = Xc[2];
    M[3] = -Xc[0] * E[0] - Xc[1] * E[1] - Xc[2] * E[2];

    M[4] = Yc[0];
    M[5] = Yc[1];
    M[6] = Yc[2];
    M[7] = -Yc[0] * E[0] - Yc[1] * E[1] - Yc[2] * E[2];

    M[8] = Zc[0];
    M[9] = Zc[1];
    M[10] = Zc[2];
    M[11] = -(Zc[0] * E[0] + Zc[1] * E[1] + Zc[2] * E[2]);

    M[12] = 0.0;
    M[13] = 0.0;
    M[14] = 0.0;
    M[15] = 1.0;
}

// modelview matrix = MCSR * Mobj
void get_modelview(double *modelview, double *object_matrix)
{
    mxm(modelview, MCSR, object_matrix);
}
// matriz de proyection (sin invertir z)
void get_projection_matrix(double *P)
{
    double n = view_near;
    double f = view_far;
    double r, t;
    int i;

    for (i = 0; i < 16; i++)
        P[i] = 0.0;

    if (paralelo == 1)
    {
        r = view_half_width;
        t = view_half_height;
        P[0] = 1.0 / r;
        P[5] = 1.0 / t;
        P[10] = 2.0 / (f - n);
        P[11] = (f + n) / (f - n);
        P[15] = 1.0;
    }
    else
    {
        // en perspectiva el volumen se mide en el plano near
        r = n * view_half_width;
        t = n * view_half_height;
        P[0] = n / r;
        P[5] = n / t;
        P[10] = (f + n) / (f - n);
        P[11] = 2.0 * f * n / (f - n);
        P[14] = -1.0; // w = -z
    }
}

// obtener la perspectiva o la vista de la cámara
void kam_ikuspegia_lortu(object3d *optr)
{
    int i;
    double modelview[16];
    double z;

    get_modelview(modelview, optr->mptr->m);

    for (i = 0; i < optr->num_vertices; i++)
    {
        // coordenadas del vertice en el sistema de la camara
        mxp(&optr->vertex_table[i].camcoord, modelview, optr->vertex_table[i].coord);

        // proyeccion paralela: x e y al rango [-1, 1]
        // proyectar (paralela o perspectiva); mxp divide entre w
        mxp(&optr->vertex_table[i].proedcoord, projection_matrix, optr->vertex_table[i].camcoord);

        // TODO modificar
        // obtener el vector normal en las coordenadas de la cámara
        optr->vertex_table[i].Ncam[0] = optr->vertex_table[i].N[0];
        optr->vertex_table[i].Ncam[1] = optr->vertex_table[i].N[1];
        optr->vertex_table[i].Ncam[2] = optr->vertex_table[i].N[2];
    }

    // TODO obtener la normal de la cara en el sistema de coordenadas del observador
    for (i = 0; i < optr->num_faces; i++)
    {
        // TODO modificar
        optr->face_table[i].Ncam[0] = optr->face_table[i].N[0];
        optr->face_table[i].Ncam[1] = optr->face_table[i].N[1];
        optr->face_table[i].Ncam[2] = optr->face_table[i].N[2];
    }
}

void modelview_lortu(double *m1, double *m2)
{
}

void mesa_lortu(double *M)
{
}

void argien_kalkulua_egin(object3d *optr, int ti)
{
}

void draw_edge(object3d *optr, int ind1, int ind2, int atzeaurpegiada)
{
    unsigned char *colorv;

    // las caras traseras (back-faces) se dibujan en rojo
    // el color de los objetos sin textura está en optr->rgb
    // si el objeto tiene textura, cada píxel debe calcular su color: ¡llamar a color_textura(u,v) para obtenerlo!
    if (atzeaurpegiada)
    {                          // sabemos que las caras traseras deben dibujarse en rojo, porque si atzearpegiakmarraztu es 0,
                               // la función dibujar_poligono termina sin dibujar nada. Por lo tanto, no llama a
                               // dibujar_triangulo, que es quien llama a draw_edge(). Así que estamos aquí porque las caras traseras deben dibujarse en rojo.
        glColor3ub(255, 0, 0); // rojo
    }
    else
    {
        // si el objeto no tiene textura, el color a usar es el color del objeto
        if (optr->texturaduna)
        {
            // obtener el color correspondiente a las coordenadas (u,v) del primer punto
            // usaremos estas coordenadas para obtener su color en la textura.
            colorv = color_textura(optr->vertex_table[ind1].u, optr->vertex_table[ind1].v);
            // establecer el color que se usará para dibujar el/los siguiente(s) píxel(es)
            glColor3ub(colorv[0], colorv[1], colorv[2]);
        }
        else
        {
            // si el objeto no tiene ninguna textura, usar el color del objeto.
            glColor3ub(optr->rgb.r, optr->rgb.g, optr->rgb.b);
        }
    }

    // dibujar los puntos internos de la arista.
    point3 p1, p2;
    float dx, dy;
    float distancia;
    float pixeldist;
    int pasos, i;
    float t, paso_t;
    float x, y, z;
    float u, v;

    // ahora hay que dibujar los píxeles entre el vértice ind1 y el vértice ind2...
    // usar coordenadas baricéntricas para interpolar los puntos de la arista.
    // las coordenadas a tener en cuenta son:
    p1 = optr->vertex_table[ind1].proedcoord;
    p2 = optr->vertex_table[ind2].proedcoord;

    // largo de aristas
    dx = p2.x - p1.x;
    dy = p2.y - p1.y;

    // distancia
    distancia = sqrt(dx * dx + dy * dy);

    // cantidad de pixeles
    pixeldist = 2.0 / (float)dimentsioa;
    pasos = (int)(distancia / pixeldist);

    if (pasos < 1)
        pasos = 1;

    paso_t = 1.0f / pasos;

    glBegin(GL_POINTS);

    for (i = 0, t = 0.0f; i <= pasos; i++, t += paso_t)
    {
        if (i == pasos)
            t = 1.0f;

        x = (1.0f - t) * p1.x + t * p2.x;
        y = (1.0f - t) * p1.y + t * p2.y;
        z = (1.0f - t) * p1.z + t * p2.z;

        if (optr->texturaduna && !atzeaurpegiada)
        {
            u = (1.0f - t) * optr->vertex_table[ind1].u + t * optr->vertex_table[ind2].u;

            v = (1.0f - t) * optr->vertex_table[ind1].v + t * optr->vertex_table[ind2].v;

            colorv = color_textura(u, v);
            glColor3ub(colorv[0], colorv[1], colorv[2]);
        }
        glVertex3f(x, y, z);
    }
    glEnd();
}

/* fidx es el índice de la cara
** i1: si la cara tiene más de 3 vértices, habrá más de 1 triángulo,
** i1 indica el triángulo i-ésimo de la cara:
**  a.- El primer vértice y los dos vértices siguientes forman el primer triángulo, por lo que
        0, 1, 2 son los índices de los vértices e i1 será 1
    b.- el primer vértice y el tercer y cuarto vértices forman el siguiente triángulo. Así,
        0, 2, 3 son los índices y, por lo tanto, i1 será 2 (segundo triángulo de la
        cara)
    c.- 0, 3, 4 forman el siguiente triángulo y así i1 = 3...
** atzeaurpegiada indica que la cara es una cara trasera (backface). Según el estado de la aplicación, la cara se dibujará en rojo o no se dibujará
*/
void dibujar_triangulo(
    object3d *optr,
    int fidx,
    int i1,
    int atzeaurpegiada)
{
    int index0, index1, index2;
    int topIndex, middleIndex, bottomIndex;
    int tempIndex;
    int rows, pixels;
    int i, j;

    point3 *top;
    point3 *middle;
    point3 *bottom;
    point3 *tempPoint;

    float pixelSize;
    float t, s, q;
    float tStep, qStep;
    float x1, x2;
    float z1, z2;
    float x, y, z;
    float alpha, beta, gamma;
    float u, v;

    unsigned char *textureColor;

    index0 = optr->face_table[fidx].vertex_ind_table[0];
    index1 = optr->face_table[fidx].vertex_ind_table[i1];
    index2 = optr->face_table[fidx].vertex_ind_table[i1 + 1];

    top = &(optr->vertex_table[index0].proedcoord);
    middle = &(optr->vertex_table[index1].proedcoord);
    bottom = &(optr->vertex_table[index2].proedcoord);

    topIndex = index0;
    middleIndex = index1;
    bottomIndex = index2;

    // Ordenar los puntos comparando cada y

    if (top->y < middle->y)
    {
        tempPoint = top;
        top = middle;
        middle = tempPoint;

        tempIndex = topIndex;
        topIndex = middleIndex;
        middleIndex = tempIndex;
    }

    if (top->y < bottom->y)
    {
        tempPoint = top;
        top = bottom;
        bottom = tempPoint;

        tempIndex = topIndex;
        topIndex = bottomIndex;
        bottomIndex = tempIndex;
    }

    if (middle->y < bottom->y)
    {
        tempPoint = middle;
        middle = bottom;
        bottom = tempPoint;

        tempIndex = middleIndex;
        middleIndex = bottomIndex;
        bottomIndex = tempIndex;
    }

    // dibujar los bordes con draw_edge()
    draw_edge(optr, topIndex, middleIndex, atzeaurpegiada);
    draw_edge(optr, middleIndex, bottomIndex, atzeaurpegiada);
    draw_edge(optr, topIndex, bottomIndex, atzeaurpegiada);

    if (lineak == 1)
        return;

    if (top->y == bottom->y)
        return;

    pixelSize = 2.0f / (float)dimentsioa;

    if (atzeaurpegiada)
        glColor3ub(255, 0, 0);
    else
        glColor3ub(optr->rgb.r, optr->rgb.g, optr->rgb.b);

    glBegin(GL_POINTS);

    // primera mitad top a middle
    if (top->y != middle->y)
    {
        rows = (int)ceilf((top->y - middle->y) / pixelSize);

        if (rows < 1)
            rows = 1;

        tStep = 1.0f / (float)rows;

        for (i = 0, t = 0.0f; i <= rows; i++, t += tStep)
        {
            if (i == rows)
                t = 1.0f;

            y = (1.0f - t) * top->y + t * middle->y;

            x1 = (1.0f - t) * top->x + t * middle->x;

            z1 = (1.0f - t) * top->z + t * middle->z;

            s = (y - top->y) / (bottom->y - top->y);

            x2 = (1.0f - s) * top->x + s * bottom->x;

            z2 = (1.0f - s) * top->z + s * bottom->z;

            pixels = (int)ceilf(fabsf(x2 - x1) / pixelSize);

            if (pixels < 1)
                pixels = 1;

            qStep = 1.0f / (float)pixels;

            for (j = 0, q = 0.0f; j <= pixels; j++, q += qStep)
            {
                if (j == pixels)
                    q = 1.0f;

                x = (1.0f - q) * x1 + q * x2;
                z = (1.0f - q) * z1 + q * z2;

                alpha = (1.0f - q) * (1.0f - t) + q * (1.0f - s);

                beta = (1.0f - q) * t;

                gamma = q * s;

                if (optr->texturaduna && !atzeaurpegiada)
                {
                    u =
                        alpha * optr->vertex_table[topIndex].u +
                        beta * optr->vertex_table[middleIndex].u +
                        gamma * optr->vertex_table[bottomIndex].u;

                    v =
                        alpha * optr->vertex_table[topIndex].v +
                        beta * optr->vertex_table[middleIndex].v +
                        gamma * optr->vertex_table[bottomIndex].v;

                    textureColor = color_textura(u, v);

                    glColor3ub(
                        textureColor[0],
                        textureColor[1],
                        textureColor[2]);
                }

                glVertex3f(x, y, z);
            }
        }
    }

    // segunda mitad middle a bottom
    if (middle->y != bottom->y)
    {
        rows = (int)ceilf((middle->y - bottom->y) / pixelSize);

        if (rows < 1)
            rows = 1;

        tStep = 1.0f / (float)rows;

        for (i = 0, t = 0.0f; i <= rows; i++, t += tStep)
        {
            if (i == rows)
                t = 1.0f;

            y = (1.0f - t) * middle->y + t * bottom->y;

            x1 = (1.0f - t) * middle->x + t * bottom->x;

            z1 = (1.0f - t) * middle->z + t * bottom->z;

            s = (y - top->y) / (bottom->y - top->y);

            x2 = (1.0f - s) * top->x + s * bottom->x;

            z2 = (1.0f - s) * top->z + s * bottom->z;

            pixels = (int)ceilf(fabsf(x2 - x1) / pixelSize);

            if (pixels < 1)
                pixels = 1;

            qStep = 1.0f / (float)pixels;

            for (j = 0, q = 0.0f; j <= pixels; j++, q += qStep)
            {
                if (j == pixels)
                    q = 1.0f;

                x = (1.0f - q) * x1 + q * x2;
                z = (1.0f - q) * z1 + q * z2;

                alpha = q * (1.0f - s);

                beta = (1.0f - q) * (1.0f - t);

                gamma = (1.0f - q) * t + q * s;

                if (optr->texturaduna && !atzeaurpegiada)
                {
                    u =
                        alpha * optr->vertex_table[topIndex].u +
                        beta * optr->vertex_table[middleIndex].u +
                        gamma * optr->vertex_table[bottomIndex].u;

                    v =
                        alpha * optr->vertex_table[topIndex].v +
                        beta * optr->vertex_table[middleIndex].v +
                        gamma * optr->vertex_table[bottomIndex].v;

                    textureColor = color_textura(u, v);

                    glColor3ub(
                        textureColor[0],
                        textureColor[1],
                        textureColor[2]);
                }

                glVertex3f(x, y, z);
            }
        }
    }
    glEnd();
}

void dibujar_poligono(object3d *optr, int ti)
{
    int i, ind0, ind1, ind2;
    int atzeaurpegiada;
    double *Nkam;

    if (ti >= optr->num_faces)
        return;
    // calcular la visibilidad con los primeros tres vértices.
    ind0 = optr->face_table[ti].vertex_ind_table[0];
    ind1 = optr->face_table[ti].vertex_ind_table[1];
    ind2 = optr->face_table[ti].vertex_ind_table[2];

    // TODO decidir si hay que dibujar o no: no dibujar lo que quede fuera del volumen de vista
    // no dibujar el poligono si algun vertice esta detras del plano near
    for (i = 0; i < optr->face_table[ti].num_vertices; i++)
    {
        if (optr->vertex_table[optr->face_table[ti].vertex_ind_table[i]].camcoord.z > -view_near)
            return;
    }

    atzeaurpegiada = 0;
    // TODO ¿es cara trasera?
    // atzeaurpegiada = ...
    if ((!atzearpegiakmarraztu) && atzeaurpegiada)
    {
        // Eliminación de caras traseras (back culling)...
        return;
    }
    if (optr->texturaduna == 0)
    {
        // TODO calcular el color de cada vértice: según las luces, la cámara y la orientación del objeto.
        argien_kalkulua_egin(optr, ti);
    }
    // si ha llegado hasta aquí, debo dibujar todos sus triángulos
    for (i = 1; i < (optr->face_table[ti].num_vertices - 1); i++) // dibujar por triángulos: con 4 vértices, dos triángulos; con cinco, 3...
        dibujar_triangulo(optr, ti, i, atzeaurpegiada);
}

static void marraztu(void)
{
    float u, v;
    int i, j;
    object3d *auxptr;
    double Fokudir[3];

    // no se puede dibujar sin objetos
    if (foptr == 0)
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glFlush();
        return;
    }

    // limpiar el viewport...
    if (objektuak == 1)
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    else
    {
        if (denak == 0)
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    }
    // TODO obtener la matriz que pasa al sistema de referencia del observador o de la cámara
    get_mcsr(MCSR);
    get_projection_matrix(projection_matrix);

    // TODO actualizar la información que tienen las luces en la vista de la cámara (tanto posiciones como direcciones)

    if (objektuak == 1)
    {
        if (denak == 1)
        {
            // printf("objektuak marraztera\n");
            for (auxptr = foptr; auxptr != 0; auxptr = auxptr->hptr)
            {
                // TODO indicar en la propia estructura del objeto cómo lo ve la cámara.
                kam_ikuspegia_lortu(auxptr);
                // printf("objektua kameraren ikuspegian daukat\n");
                for (i = 0; i < auxptr->num_faces; i++)
                {
                    dibujar_poligono(auxptr, i);
                }
            }
        }
        else
        {
            // TODO indicar en la propia estructura del objeto cómo lo ve la cámara.
            kam_ikuspegia_lortu(sel_ptr);
            for (i = 0; i < sel_ptr->num_faces; i++)
            {
                dibujar_poligono(sel_ptr, i);
            }
        }
    }
    else
    {
        // TODO indicar en la propia estructura del objeto cómo lo ve la cámara.
        kam_ikuspegia_lortu(sel_ptr);
        dibujar_poligono(sel_ptr, indexx);
    }
    glFlush();
}

void obj_normalak_kalkulatu(object3d *optr)
{
}

void read_from_file(char *fitx, object3d **foptrptr)
{
    int i, retval;
    object3d *optr;

    printf("%s fitxategitik datuak hartzera\n", fitx);
    optr = (object3d *)malloc(sizeof(object3d));
    optr->rgb.r = 0;
    // int read_wavefront(char * file_name, object3d * object_ptr) {
    retval = read_wavefront(fitx, optr);
    if (retval != 0)
    {
        printf("%s fitxategitik datuak hartzerakoan arazoak izan ditut\n", fitxiz);
        free(optr);
    }
    else
    {

        printf("Matriz del objeto...\n");
        optr->mptr = (mlist *)malloc(sizeof(mlist));
        for (i = 0; i < 16; i++)
            optr->mptr->m[i] = 0;
        optr->mptr->m[0] = 1.0;
        optr->mptr->m[5] = 1.0;
        optr->mptr->m[10] = 1.0;
        optr->mptr->m[15] = 1.0;
        optr->mptr->hptr = 0;

        // printf("La información se dirige a la lista de objetos o cámaras...\n");
        optr->hptr = *foptrptr;
        *foptrptr = optr;
        // printf("calcular las normales\n");
        // for (i = 0; i< optr->num_vertices; i++) printf("%lf %lf %lf\n", optr->vertex_table[i].coord.x,optr->vertex_table[i].coord.y,optr->vertex_table[i].coord.z);
        obj_normalak_kalkulatu(optr);
        sel_ptr = optr;
        if (optr->texturaduna && (bufferra == 0))
        {
            // colocamos la información de la textura en el buffer señalado por bufferra. Las dimensiones de la textura se cargan en dimx y dimy
            retval = load_ppm("discretizacionLPF.ppm", &bufferra, &dimx, &dimy);
            if (retval != 1)
            {
                printf("No hay archivo de textura (discretizacionLPF.ppm)\n");
                optr->texturaduna = 0;
                // exit(-1);
            }
        }
    }
    // printf("datuak irakurrita\n");
}
// obtener el centro del objeto que esta seleccionado en coordenadas del mundo
void get_object_center(point3 *center)
{
    point3 local_center;

    // calcular el centro de la caja que esta envolviendo al objeto seleccionado
    local_center.x = (sel_ptr->max.x + sel_ptr->min.x) / 2.0;
    local_center.y = (sel_ptr->max.y + sel_ptr->min.y) / 2.0;
    local_center.z = (sel_ptr->max.z + sel_ptr->min.z) / 2.0;

    // pasarla al mundo
    mxp(center, sel_ptr->mptr->m, loca_center);
}

// girar la camara alrededor del centro del objeto: M_cam · T(Cc) · R · T(-Cc)
void orbit_camera_around_object(double *R)
{
    point3 center;
    // centro del objeto visto desde la camara
    point3 cc;
    double T[16], T_inv[16], tmp[16], total[16];
    int i;

    // obtener el centro del objeto en coordenadas del mundo
    get_object_center(&center);
    get_mcsr(MCSR);
    mxp(&cc, MCSR, center);

    // trabajar la matriz de transformación: T(Cc) · R · T(-Cc)
    for (i = 0; i < 16; i++)
    {
        T[i] = 0.0;
        T_inv[i] = 0.0;
    }
    T[0] = T[5] = T[10] = T[15] = 1.0;
    T_inv[0] = T_inv[5] = T_inv[10] = T_inv[15] = 1.0;
    T[3] = cc.x;
    T[7] = cc.y;
    T[11] = cc.z;
    T_inv[3] = -cc.x;
    T_inv[7] = -cc.y;
    T_inv[11] = -cc.z;

    mxm(tmp, T, R);
    mxm(total, tmp, T_inv);
    kamerari_aldaketa_sartu_esk(total); // local
}

void x_aldaketa(int dir)
{

    if (kamera == 0) // estoy modificando el objeto
    {
        if (aldaketa == 'r')
        {
            // rotar cos(5) = 0.99619469809174;  // cos(5)
            if (sel_ptr != 0)
            {

                double c = 0.99619469809174;
                double s = (dir == 1) ? 0.087155742747658 : -0.087155742747658; // sin(5)
                double m[16] = {
                    1.0, 0.0, 0.0, 0.0,
                    0.0, c, -s, 0.0,
                    0.0, s, c, 0.0,
                    0.0, 0.0, 0.0, 1.0};
                if (ald_lokala == 1)
                {
                    objektuari_aldaketa_sartu_esk(m); // local
                }
                else
                {
                    objektuari_aldaketa_sartu_ezk(m); // global
                }
            }
        }
        else
        {
            // trasladar en x
            //'x' positio
            //'X' negativo

            if (sel_ptr != 0)
            {
                double step = (dir == 1) ? 0.02 : -0.02;
                double m[16] = {
                    1.0, 0.0, 0.0, step,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 1.0, 0.0,
                    0.0, 0.0, 0.0, 1.0};
                if (ald_lokala == 1)
                {
                    objektuari_aldaketa_sartu_esk(m); // local
                }
                else
                {
                    objektuari_aldaketa_sartu_ezk(m); // global
                }
            }
        }
    }
    else if (kamera == 1) // estoy modificando la cámara
    {
        if (ald_lokala == 1) // en modo vuelo, mirar izquierda/derecha (rotación respecto a y)
        {
            double c = 0.99619469809174;
            double s = (dir == 1) ? -0.087155742747658 : 0.087155742747658; // sin(-5) o sin(5)
            double m[16] = {
                c, 0.0, s, 0.0,
                0.0, 1.0, 0.0, 0.0,
                -s, 0.0, c, 0.0,
                0.0, 0.0, 0.0, 1.0};
            kamerari_aldaketa_sartu_esk(m); // local
        }
        else // en modo análisis
        {
            if (sel_ptr != 0)
            {
                // analizar el objeto seleccionado más hacia la derecha o hacia la izquierda (¡girando!)
                // orbitar alrededor del eje y: 'x' la camara va a la derecha, 'X' a la izquierda
                double c = 0.99619469809174;
                double s = (dir == 1) ? 0.087155742747658 : -0.087155742747658;
                double R[16] = {
                    c, 0.0, s, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    -s, 0.0, c, 0.0,
                    0.0, 0.0, 0.0, 1.0};

                orbit_camera_around_object(R);
            }
        }
    }
    else // modificando las luces
    {
        // girar el sol o mover la bombilla
    }
}

void y_aldaketa(int dir)
{

    if (kamera == 0) // estoy modificando el objeto
    {
        if (aldaketa == 'r')
        {
            // rotar cos(5) = 0.99619469809174;  // cos(5)
            if (sel_ptr != 0)
            {

                double c = 0.99619469809174;
                double s = (dir == 1) ? 0.087155742747658 : -0.087155742747658; // sin(5)
                double m[16] = {
                    c, 0.0, s, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    -s, 0.0, c, 0.0,
                    0.0, 0.0, 0.0, 1.0};
                if (ald_lokala == 1)
                {
                    objektuari_aldaketa_sartu_esk(m); // local
                }
                else
                {
                    objektuari_aldaketa_sartu_ezk(m); // global
                }
            }
        }
        else
        {
            // trasladar en y
            //'y' positivo
            //'Y' negativo

            if (sel_ptr != 0)
            {
                double step = (dir == 1) ? 0.02 : -0.02;
                double m[16] = {
                    1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, step,
                    0.0, 0.0, 1.0, 0.0,
                    0.0, 0.0, 0.0, 1.0};
                if (ald_lokala == 1)
                {
                    objektuari_aldaketa_sartu_esk(m); // local
                }
                else
                {
                    objektuari_aldaketa_sartu_ezk(m); // global
                }
            }
        }
    }
    else if (kamera == 1) // estoy modificando la cámara
    {
        if (ald_lokala == 1) // en modo vuelo, mirar izquierda/derecha (rotación respecto a y)
        {
            // girar alrededor del eje x de la camara
            double c = 0.99619469809174;
            double s = (dir == 1) ? 0.087155742747658 : -0.087155742747658; // sin(5) o sin(-5)
            double m[16] = {
                1.0, 0.0, 0.0, 0.0,
                0.0, c, -s, 0.0,
                0.0, s, c, 0.0,
                0.0, 0.0, 0.0, 1.0};

            kamerari_aldaketa_sartu_esk(m);
        }
        else // en modo análisis
        {
            if (sel_ptr != 0)
            {
                // analizar el objeto seleccionado más desde arriba o desde abajo (¡girando!)
                // orbitar alrededor del eje x: 'y' la camara sube, 'Y' baja
                double c = 0.99619469809174;
                double s = (dir == 1) ? -0.087155742747658 : 0.087155742747658;
                double R[16] = {
                    1.0, 0.0, 0.0, 0.0,
                    0.0, c, -s, 0.0,
                    0.0, s, c, 0.0,
                    0.0, 0.0, 0.0, 1.0};

                orbit_camera_around_object(R);
            }
        }
    }
    else // modificando las luces
    {
        // girar el sol o mover la bombilla
    }
}

void z_aldaketa(int dir)
{
    if (kamera == 0) // modificación del objeto
    {
        if (aldaketa == 'r')
        {
            // rotar cos(5) = 0.99619469809174;  // cos(5)
            if (sel_ptr != 0)
            {

                double c = 0.99619469809174;
                double s = (dir == 1) ? 0.087155742747658 : -0.087155742747658; // sin(5)
                double m[16] = {
                    c, -s, 0.0, 0.0,
                    s, c, 0.0, 0.0,
                    0.0, 0.0, 1.0, 0.0,
                    0.0, 0.0, 0.0, 1.0};

                if (ald_lokala == 1)
                {
                    objektuari_aldaketa_sartu_esk(m); // local
                }
                else
                {
                    objektuari_aldaketa_sartu_ezk(m); // global
                }
            }
        }
        else
        {
            // trasladar en z
            //'z' positivo
            //'Z' negativo
            if (sel_ptr != 0)
            {
                double step = (dir == 1) ? 0.02 : -0.02;
                double m[16] = {
                    1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 1.0, step,
                    0.0, 0.0, 0.0, 1.0};

                if (ald_lokala == 1)
                {
                    objektuari_aldaketa_sartu_esk(m); // local
                }
                else
                {
                    objektuari_aldaketa_sartu_ezk(m); // global
                }
            }
        }
    }
    else // modificación de la cámara
        if (kamera == 1)
        {
            // en modo vuelo, mover siempre la cámara hacia delante o hacia atrás.
            if (ald_lokala == 1)
            {

                double step = (dir == 1) ? -0.05 : 0.05;
                double m[16] = {
                    1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 1.0, step,
                    0.0, 0.0, 0.0, 1.0};
                kamerari_aldaketa_sartu_esk(m);
            }
            else
            {
                if (sel_ptr != 0)
                {
                    if (aldaketa == 't')
                    {
                        // acercarse ('z') o alejarse ('Z') sin atravesar el objeto
                        point3 center, cc;
                        double distance;
                        double step = (dir == 1) ? -0.05 : 0.05;
                        double m[16] = {
                            1.0, 0.0, 0.0, 0.0,
                            0.0, 1.0, 0.0, 0.0,
                            0.0, 0.0, 1.0, step,
                            0.0, 0.0, 0.0, 1.0};

                        get_object_center(&center);
                        get_mcsr(MCSR);
                        mxp(&cc, MCSR, center);
                        distance = sqrt(cc.x * cc.x + cc.y * cc.y + cc.z * cc.z);

                        if ((dir == 0) || (distance + step > view_near + 0.5)) // controlar la distancia
                            kamerari_aldaketa_sartu_esk(m);
                        else
                            printf("demasiado cerca del objeto\n");
                    }
                    else
                    {
                        // roll: girar alrededor de la linea de vision
                        double c = 0.99619469809174;
                        double s = (dir == 1) ? 0.087155742747658 : -0.087155742747658;
                        double R[16] = {
                            c, -s, 0.0, 0.0,
                            s, c, 0.0, 0.0,
                            0.0, 0.0, 1.0, 0.0,
                            0.0, 0.0, 0.0, 1.0};

                        orbit_camera_around_object(R);
                    }
                }
            }
        }
        else // modificando las luces
        {
            // ¿mover la bombilla en el mundo o girar el sol?
        }
}

void undo()
{
    mlist *top;

    if (kamera == 0)
    { // deshacer la última transformación del objeto seleccionado
        if (sel_ptr != 0)
        {
            if (sel_ptr->mptr->hptr != 0)
            { // no se quita la matriz original
                top = sel_ptr->mptr;
                sel_ptr->mptr = sel_ptr->mptr->hptr;
                free(top);
            }
            else
            {
                printf("No hay más cambios que deshacer\n");
            }
        }
    }
    else if (kamera == 1)
    {
        // undo movimiento de la camar
        if (camera_ptr->hptr != 0)
        {
            top = camera_ptr;
            camera_ptr = camera_ptr->hptr;
            free(top);
        }
        else
        {
            printf("No hay más cambios que deshacer\n");
        }
    }
}
// borrar el objeto seleccionado
// borrar el objeto seleccionado
void delete_object()
{
    object3d *prev;
    object3d *to_delete;

    if (sel_ptr == 0)
        return;

    to_delete = sel_ptr;

    // quitarlo de la lista
    if (foptr == to_delete)
        foptr = to_delete->hptr;
    else
    {
        prev = foptr;
        while (prev->hptr != to_delete)
            prev = prev->hptr;
        prev->hptr = to_delete->hptr;
    }

    // seleccionar el siguiente o el primero si era el último
    sel_ptr = to_delete->hptr;
    if (sel_ptr == 0)
        sel_ptr = foptr;
    indexx = 0;

    free(to_delete);
}

// orientar la camara para que mire al centro del objeto seleccionado (sin moverla)
void kamera_objektuari_begira()
{
    mlist *new_matrix;
    point3 center;
    double E[3], Xc[3], Yc[3], Zc[3];
    double length;

    if (sel_ptr == 0)
        return;

    get_object_center(&center);
    E[0] = camera_ptr->m[3];
    E[1] = camera_ptr->m[7];
    E[2] = camera_ptr->m[11];

    // Zc: del objeto hacia la camara (la camara mira hacia -Zc)
    Zc[0] = E[0] - center.x;
    Zc[1] = E[1] - center.y;
    Zc[2] = E[2] - center.z;
    length = sqrt(Zc[0] * Zc[0] + Zc[1] * Zc[1] + Zc[2] * Zc[2]);
    if (length < 0.000001) // la camara esta en el centro del objeto
        return;
    Zc[0] /= length;
    Zc[1] /= length;
    Zc[2] /= length;

    // Xc = arriba × Zc, con arriba = (0,1,0)
    Xc[0] = Zc[2];
    Xc[1] = 0.0;
    Xc[2] = -Zc[0];
    length = sqrt(Xc[0] * Xc[0] + Xc[2] * Xc[2]);
    if (length < 0.000001) // mirando justo hacia arriba o hacia abajo
    {
        Xc[0] = 1.0;
        Xc[2] = 0.0;
    }
    else
    {
        Xc[0] /= length;
        Xc[2] /= length;
    }

    // Yc = Zc × Xc
    Yc[0] = Zc[1] * Xc[2] - Zc[2] * Xc[1];
    Yc[1] = Zc[2] * Xc[0] - Zc[0] * Xc[2];
    Yc[2] = Zc[0] * Xc[1] - Zc[1] * Xc[0];

    // nueva matriz de la camara: columnas Xc, Yc, Zc, E
    new_matrix = (mlist *)malloc(sizeof(mlist));
    new_matrix->m[0] = Xc[0];
    new_matrix->m[1] = Yc[0];
    new_matrix->m[2] = Zc[0];
    new_matrix->m[3] = E[0];
    new_matrix->m[4] = Xc[1];
    new_matrix->m[5] = Yc[1];
    new_matrix->m[6] = Zc[1];
    new_matrix->m[7] = E[1];
    new_matrix->m[8] = Xc[2];
    new_matrix->m[9] = Yc[2];
    new_matrix->m[10] = Zc[2];
    new_matrix->m[11] = E[2];
    new_matrix->m[12] = 0.0;
    new_matrix->m[13] = 0.0;
    new_matrix->m[14] = 0.0;
    new_matrix->m[15] = 1.0;

    new_matrix->hptr = camera_ptr;
    camera_ptr = new_matrix;
}

void print_egoera()
{
    if (kamera == 0)
    {
        if (ald_lokala == 1)
            printf("\nestás modificando el objeto (transformación local)\n");
        else
            printf("\nestás modificando el objeto (transformación global)\n");
    }
    if (kamera == 1)
    {
        if (ald_lokala == 1)
            printf("\nestás modificando la cámara en modo vuelo\n");
        else
            printf("\nestás modificando la cámara en modo análisis\n");
    }
    if (kamera == 2)
    {
        printf("\nestás modificando las luces\n");
    }
    if (aldaketa == 't')
        printf("Traslación activada\n");
    else
        printf("Rotaciones activadas\n");
    if (objektuaren_ikuspegia)
        printf("estás mostrando el punto de vista del objeto (pulsa `C` para pasar al de la cámara)\n");
}

// Esta función se llamará cada vez que el usuario pulse una tecla
static void teklatua(unsigned char key, int x, int y)
{
    int retval;
    int i;
    FILE *obj_file;

    switch (key)
    {
    case 13:
        if (foptr != 0) // si no hay objeto, no debe hacer nada
        {
            indexx++; // si es el último, convertirlo en el primero (¡hay que controlarlo!)
            if (indexx == sel_ptr->num_faces)
            {
                indexx = 0;
                if ((denak == 1) && (objektuak == 0))
                {
                    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
                    glFlush();
                }
            }
        }
        break;
    case 8:
        if (foptr != 0) // si no hay objeto, no debe hacer nada
        {
            delete_object();
        }
        break;
    case 127:
        if (foptr != 0) // si no hay objeto, no debe hacer nada
        {
            delete_object();
        }
        break;

    case 'd':
        if (denak == 1)
            denak = 0;
        else
            denak = 1;
        break;
    case 'o':
        if (objektuak == 1)
            objektuak = 0;
        else
            objektuak = 1;
        break;
    case 'c':
        if (kamera == 0) // estoy modificando el objeto.
        {
            if (objektuaren_ikuspegia == 1) // si estoy en la vista del objeto, puedo pasar a modificar las luces, no a modificar la cámara.
            {
                kamera = 2;
                printf("argiak aldatzera zoaz (objektuaren ikuspegian zaude)\n");
            }
            else
            {
                kamera = 1;
                ald_lokala = 1; // me pondré en modo vuelo
                printf("kamera aldatzera zoaz (hegaldi moduan zaude)\n");
            }
        }
        else
        {
            if (kamera == 1)
            {
                kamera = 2;
                printf("argia aldatzera zoaz \n");
            }
            else
            {
                kamera = 0;
                ald_lokala = 1; // me pondré en modo vuelo
                printf("objektua aldatzera zoaz (aldaketa lokala daukazu) \n");
            }
        }
        break;
    case 'C':
        if (objektuaren_ikuspegia == 1)
            objektuaren_ikuspegia = 0;
        else
        {
            objektuaren_ikuspegia = 1;
            if (kamera == 1)
            {
                kamera = 0;
            }
            ald_lokala = 1;
            printf("objektuaren ikuspegian sartu zara eta aldaketak era lokalean eragingo dituzu \n");
        }
        break;
    case 'l':
        if (lineak == 1)
            lineak = 0;
        else
            lineak = 1;
        break;
    case 't':
        aldaketa = 't';
        printf("traslazioak\n");
        break;
    case 'r':
        aldaketa = 'r';
        printf("biraketak\n");
        break;
    case 'n':
        if (normalak_marraztu == 0)
        {
            normalak_marraztu = 1;
            printf("normalak marraztuko dira\n");
        }
        else
        {
            normalak_marraztu = 0;
            printf("normalak ez dira marraztuko\n");
        }
        break;
    case 'b':
        if (atzearpegiakmarraztu == 1)
        {
            atzearpegiakmarraztu = 0;
            printf("atze-aurpegiak ez dira marraztuko\n");
        }
        else
        {
            atzearpegiakmarraztu = 1;
            printf("atze aurpegiak gorriz marraztuko dira\n");
        }
        break;
    case 'g':
        if (objektuaren_ikuspegia == 0) // en la vista del objeto, el cambio siempre será local.
        {
            if (ald_lokala == 1)
            {
                ald_lokala = 0;
                if ((kamera == 1) && (sel_ptr != 0))
                {
                    kamera_objektuari_begira();
                    // print_matrizea16(M_kam);
                }
            }
            else
                ald_lokala = 1;
        }
        else
            printf("objektuaren ikuspegian aldaketak beti lokalak dira\n");
        break;
    case 'x':
        x_aldaketa(1);
        break;
    case 'y':
        y_aldaketa(1);
        break;
    case 'z':
        z_aldaketa(1);
        break;
    case 'X':
        x_aldaketa(0);
        break;
    case 'Y':
        y_aldaketa(0);
        break;

    case 'Z':
        z_aldaketa(0);
        break;
    case 'u':
        undo();
        break;
    case 'p':
        if (paralelo == 1)
        {
            printf("perspektiban agertu behar du\n");
            paralelo = 0;
        }
        else
        {
            printf("paraleloan agertu behar du\n");
            paralelo = 1;
        }
        break;
    case 's': // iluminación suavizada
        if (flatmode == 0)
        {
            flatmode = 1;
            printf("argiztapen finkoa poligono osoan\n");
        }
        else
        {
            flatmode = 0;
            printf("argiztapen suabizatua\n");
        }
        break;
    case '1':
        // Encender/apagar el sol
        break;
    case '2':
        // Encender/apagar la bombilla
        break;
    case '3':
        // Encender/apagar el foco del objeto
        break;
    case '4':
        // Encender/apagar el foco de la cámara
        break;
    case '+':
        if (kamera == 2)
        { // Aumentar la apertura de los focos
        }
        else
        {
            if (kamera == 1)
            { // Aumentar el volumen de vista de la cámara
                view_half_width *= 1.1;
                view_half_height *= 1.1;
            }
            else
            {
                // kamaera ==0, escalar el objeto (agrandar)
                if (sel_ptr != 0)
                {
                    double m[16] = {1.1, 0.0, 0.0, 0.0,
                                    0.0, 1.1, 0.0, 0.0,
                                    0.0, 0.0, 1.1, 0.0,
                                    0.0, 0.0, 0.0, 1.};

                    if (ald_lokala == 1)
                        // modificación local
                        objektuari_aldaketa_sartu_esk(m);
                    else
                        // modificación global
                        objektuari_aldaketa_sartu_ezk(m);
                }
            }
        }
        break;
    case '-':
        if (kamera == 2)
        { // Reducir la apertura de los focos
        }
        else
        {
            if (kamera == 1)
            { // Reducir el volumen de vista de la cámara
                // no dejar que reduzca demasiado, no puede ser 0 porque se usa para dividir
                if (view_half_width > 0.05)
                {
                    view_half_width /= 1.1;
                    view_half_height /= 1.1;
                }
            }
            else
            {
                // escalar el objeto (achicar)
                if (sel_ptr != 0)
                {
                    double s = 1.0 / 1.1;
                    double m[16] =
                        {s, 0.0, 0.0, 0.0,
                         0.0, s, 0.0, 0.0,
                         0.0, 0.0, s, 0.0,
                         0.0, 0.0, 0.0, 1.};

                    if (ald_lokala == 1)
                        // modificación local
                        objektuari_aldaketa_sartu_esk(m);
                    else
                        // modificación global
                        objektuari_aldaketa_sartu_ezk(m);
                }
            }
        }
        break;
    case 'f':
        /* Pedir archivo */
        printf("idatzi fitxategi izena\n");
        scanf("%s", &(fitxiz[0]));
        read_from_file(fitxiz, &foptr);
        indexx = 0;
        if ((ald_lokala == 0) && (kamera == 1) && (sel_ptr != 0))
            kamera_objektuari_begira();
        break;
    case 9:             /* <TAB> */
        if (foptr != 0) // si no hay objeto, no debe hacer nada
        {
            sel_ptr = sel_ptr->hptr;
            /* La selección es circular, así que si salimos de la lista volvemos al primer elemento */
            if (sel_ptr == 0)
                sel_ptr = foptr;
            indexx = 0; // el polígono seleccionado es el primero
            if ((ald_lokala == 0) && (kamera == 1))
            {
                // ¡hay que poner la cámara mirando al objeto!!
                kamera_objektuari_begira();
            }
        }
        break;
    case 27: // <ESC>
        exit(0);
        break;
    default:
        printf("%d %c\n", key, key);
    }
    print_egoera();
    // Hay que redibujar la pantalla para mostrar el nuevo triángulo
    glutPostRedisplay();
}

void viewportberria(int zabal, int garai)
{
    if (zabal < garai)
        dimentsioa = zabal;
    else
        dimentsioa = garai;
    glViewport(0, 0, dimentsioa, dimentsioa);
    printf("nuevo número de líneas = %d\n", dimentsioa);
}

int main(int argc, char **argv)
{
    int retval, i;

    printf(" Triángulos con puntos internos y textura \n");
    printf("Press <ESC> to finish\n");
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH);
    dimentsioa = 500;
    glutInitWindowSize(dimentsioa, dimentsioa);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("KBG/GO praktika");

    glutDisplayFunc(marraztu);
    glutKeyboardFunc(teklatua);
    glutReshapeFunc(viewportberria);
    /* we put the information of the texture in the buffer pointed by bufferra. The dimensions of the texture are loaded into dimx and dimy
        retval = load_ppm("testura.ppm", &bufferra, &dimx, &dimy);
        if (retval !=1)
            {
            printf("Ez dago testuraren fitxategia (testura.ppm)\n");
            exit(-1);
            }
             */
    bufferra = 0;
    dimx = 0;
    dimy = 0;
    glClearColor(0.0f, 0.0f, 0.7f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST); // activar el test de profundidad (Z-buffer)
    glDepthFunc(GL_GREATER);
    glClearDepth(0.0); // dibujar primero el más lejano (el mayor)
    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    // glOrtho(-1.0, 1.0, -1.0, 1.0, 1.0, -1.0);
    // glMatrixMode(GL_MODELVIEW);
    denak = 1;
    lineak = 0;
    objektuak = 1;
    kamera = 0;
    foptr = 0;
    sel_ptr = 0;
    aldaketa = 'r';
    ald_lokala = 1;
    objektuaren_ikuspegia = 0;
    normalak_marraztu = 0;
    flatmode = 0;
    paralelo = 1;
    atzearpegiakmarraztu = 1;
    // nicializar la cámara: al principio está en la posición (0,0,2.5)
    camera_ptr = (mlist *)malloc(sizeof(mlist));
    for (i = 0; i < 16; i++)
    {
        camera_ptr->m[i] = 0.0;
    }
    camera_ptr->m[0] = 1.0;
    camera_ptr->m[5] = 1.0;
    camera_ptr->m[10] = 1.0;
    camera_ptr->m[15] = 1.0;
    camera_ptr->m[11] = 2.5;
    camera_ptr->hptr = 0;

    // volumen
    view_half_width = 1.0;
    view_half_height = 1.0;
    view_near = 1.0;
    view_far = 10.0;

    // TODO Inicializar las luces

    if (argc > 1)
        read_from_file(argv[1], &foptr);
    else
    {

        read_from_file("abioia-1+1.obj", &foptr);
        if (sel_ptr != 0)
        {
            sel_ptr->mptr->m[3] = -1.0;
        }
        read_from_file("abioia-1+1.obj", &foptr);
        if (sel_ptr != 0)
        {
            sel_ptr->mptr->m[3] = 1.0;
        }
        read_from_file("abioia-1+1.obj", &foptr);
        if (sel_ptr != 0)
        {
            sel_ptr->mptr->m[7] = -0.4;
            sel_ptr->mptr->m[11] = 0.5;
        }
        read_from_file("abioia-1+1.obj", &foptr);
        if (sel_ptr != 0)
        {
            sel_ptr->mptr->m[11] = -0.7;
        }
        read_from_file("abioia-1+1.obj", &foptr);
        if (sel_ptr != 0)
        {
            sel_ptr->mptr->m[7] = 0.7;
        }

        // read_from_file("z-1+1.obj",&foptr);
        // read_from_file("abioia-1+1.obj",&foptr);
        // read_from_file("triangles-1+1.obj",&foptr);
    }
    print_egoera();
    glutMainLoop();

    return 0;
}
