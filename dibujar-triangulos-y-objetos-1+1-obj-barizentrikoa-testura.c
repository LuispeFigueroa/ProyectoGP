//	Program developed by
//
//	Informatika Fakultatea
//	Euskal Herriko Unibertsitatea
//	http://www.ehu.eus/if
//
// to compile it: gcc dibujar-triangulos-y-objetos.c cargar-ppm.c load_obj_joseba.c -lGL -lGLU -lglut -lm
//
//  or simply:    gcc *.c -lGL -lGLU -lglut -lm
//

#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
// #include "cargar-triangulo.h"
#include "obj.h"

// testuraren informazioa
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

// TODO  m = m1 * m2
// m1 bider m2 matrizeen biderketa egin eta m matrizean jaso.
void mxm(double *m, double *m1, double *m2)
{
}

// TODO
void kamerari_aldaketa_sartu_ezk(double *m)
{
}

// TODO
void kamerari_aldaketa_sartu_esk(double m[16])
{
}

// TODO
void objektuari_aldaketa_sartu_ezk(double m[16])
{
}

// TODO
void objektuari_aldaketa_sartu_esk(double m[16])
{
}

// TODO given u,v get the color pointer
unsigned char *color_textura(float u, float v)
{
    int indx, indy;
    char *lag;
    // printf("texturan...%x\n",bufferra);
    // TODO get the desplacement for indx and indy
    //  negative values?
    //  values greater than 1?
    indx = 0;
    indy = 0;
    lag = (unsigned char *)bufferra;
    // printf("irtetera %x\n",lag[indy*dimx+indx]);
    return (lag + 3 * (indy * dimx + indx));
}

void print_matrizea16(double *m)
{
    int i;

    for (i = 0; i < 4; i++)
        printf("%lf, %lf, %lf, %lf\n", m[i * 4], m[i * 4 + 1], m[i * 4 + 2], m[i * 4 + 3]);
}

// TODO  res = m * v
// v bektoreari m matrizea bidertu eta res erakusleak adierazten duen bektorean jaso.
// v bektorearen eta res emaitzaren laugarren osagaia 0 dela suposatzen du.
void mxv(double *res, double *m, double *v)
{
    res[0] = v[0];
    res[1] = v[1];
    res[2] = v[2];
}

// TODO  pptr = m * p
// ppuntuari m matrizea bidertu eta pptr erakulseak adierazten duen puntuan jaso.
// p puntuaren laugarren osagaia 1 dela suposatzen du.
// matrizearen laugarren lerroaren arabera emaitzaren laugarren osagaia, w, ez bada 1, orduan bere baliokidea itzuli behar du: x/w, y/w eta z/w
void mxp(point3 *pptr, double m[16], point3 p)
{
    pptr->x = p.x;
    pptr->y = p.y;
    pptr->z = p.z;
}

// TODO objektuaren erpinek eta bektore normalek kameraren erreferentzi-sisteman dituzten koordenatuak lortu
void kam_ikuspegia_lortu(object3d *optr)
{
    int i;

    // TODO  get point in the viewer coordenate-system and project it
    // TODO get the vectors in camera system-
    for (i = 0; i < optr->num_vertices; i++)
    {
        // TODO aldatu
        //  get viewer coordinates
        optr->vertex_table[i].camcoord.x = optr->vertex_table[i].coord.x;
        optr->vertex_table[i].camcoord.y = optr->vertex_table[i].coord.y;
        optr->vertex_table[i].camcoord.z = optr->vertex_table[i].coord.z;
        // TODO aldatu
        // Get projected coordinates
        optr->vertex_table[i].proedcoord.x = optr->vertex_table[i].camcoord.x;
        optr->vertex_table[i].proedcoord.y = optr->vertex_table[i].camcoord.y;
        optr->vertex_table[i].proedcoord.z = optr->vertex_table[i].camcoord.z;
        // TODO aldatu
        // get normal vector in camera coordinates
        optr->vertex_table[i].Ncam[0] = optr->vertex_table[i].N[0];
        optr->vertex_table[i].Ncam[1] = optr->vertex_table[i].N[1];
        optr->vertex_table[i].Ncam[2] = optr->vertex_table[i].N[2];
    }

    // TODO get the face normal in the viewer coordenate-system
    for (i = 0; i < optr->num_faces; i++)
    {
        // TODO aldatu
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

    // backfaces are drawn in red
    // the colour for object without texture are in optr->rgb
    // if the object has texture each pixel must compute the colour: call to color_textura(u,v) to get it!
    if (atzeaurpegiada)
    {                          // we know that back-faces must be drawn in red, because if atzeaurpegiakmarraztu is 0,
                               // the function dibujar_poligono finishes withow drawing anything. And so, it does not call to
                               // dibujar_triangulo  wich calls to draw_edge(). So we are here because back-faces must be drawn in red.
        glColor3ub(255, 0, 0); // red
    }
    else
    {
        // if the object does not have texture, then the colour to be used is the colour of the object
        if (optr->texturaduna)
        {
            // get the colour corresponding to (u,v) coordinates of the first point
            // we will use these coordinates to catch its colour in the texture.
            colorv = color_textura(optr->vertex_table[ind1].u, optr->vertex_table[ind1].v);
            // set the colour to be used to draw next pixel(s)
            glColor3ub(colorv[0], colorv[1], colorv[2]);
        }
        else
        {
            // if the object does not have any texture use the colour of the object.
            glColor3ub(optr->rgb.r, optr->rgb.g, optr->rgb.b);
        }
    }

    // draw the internal points of the edge.
    point3 p1, p2;
    float dx, dy;
    float distancia;
    float pixeldist;
    int pasos, i;
    float t, paso_t;
    float x, y, z;
    float u, v;

    // now we have to draw the pixels between ind1 vertex and ind2 vertex...
    // use barycentric coordinates to interpolate the points in the edge.
    // the coordinates to be taken into account are:
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

/* fidx is the face index
** i1 if the face has more than 3 vertices there will be more than 1 triangle,
** i1 indicates the ith triangle of the face:
**  a.- The first vertex and the next two vertices form the first triangle, so
        0, 1, 2 are the indices of the vertices and i1 will be 1
    b.- the first vertex and the third and fourth vertices form the next triangle. So
        0, 2, 3 are the indices and consequently 1i will be 2 (second triangle of the
        face)
    c.- 0, 3, 4 form the next triangle and so i1= 3...
** atzeaurpegiada indicates that the face is a backface. Depending of the state of the aplication tha face will be drawn in red or it will not be drawn
*/
void dibujar_triangulo(object3d *optr, int fidx, int i1, int atzeaurpegiada)
{
    point3 *pgoiptr, *pbeheptr, *perdiptr;
    float x, y, z, u, v, nx, ny;
    float luz, l12, l23, l13, d1v23, d2v13, d3v12;
    float lerrotartea, pixeldist;
    float alfa, beta, gamma;
    float goieragina, beheeragina, erdieragina;
    float *luzeptr, *erdiptr, *motzptr;
    float luzeluz, paraleloarenluzera;
    int luzeanparalelokop, barnekop;
    int lerrokop, i, j;
    int ind0, ind1, ind2, indg, inde, indb;
    point3 *p1ptr, *p2ptr, *p3ptr;
    double *Nkam;
    double aldaketa, baldaketa;
    float x1, x2, y1;
    unsigned char r, g, b;
    unsigned char *colorv;

    // triangeluaren hiru erpinak hartu
    ind0 = optr->face_table[fidx].vertex_ind_table[0];
    ind1 = optr->face_table[fidx].vertex_ind_table[i1];
    ind2 = optr->face_table[fidx].vertex_ind_table[i1 + 1];

    p1ptr = &(optr->vertex_table[ind0].proedcoord);
    p2ptr = &(optr->vertex_table[ind1].proedcoord);
    p3ptr = &(optr->vertex_table[ind2].proedcoord);

    // Puntuz puntu marraztuko dut dena!!
    // hasteko bi pixelen arteko distantzia gure munduan (-1 eta 1 arteko munduan) ze distantzia den kalkulatuko dut
    pixeldist = 2.0 / (float)dimentsioa;

    // lehenengo hiru erpinak ordenatu behar ditut
    // TODO erpinak ordenatu!!!
    pgoiptr = p1ptr;
    perdiptr = p2ptr;
    pbeheptr = p3ptr;
    indg = ind0;
    inde = ind1;
    indb = ind2;
    // take the colour of the object
    r = optr->rgb.r; // from double to unsigned char!
    g = optr->rgb.g; // from double to unsigned char!
    b = optr->rgb.b; // from double to unsigned char!
    // set or change the colour that will be used to draw
    glColor3ub(r, g, b);

    // TODO change all!!! initially the code only draws 3 points (the vertices of the triangle).
    //      nothing more is drawn.
    //  draw the three vertices.
    //  3 erpinak marraztu
    glBegin(GL_POINTS);
    glColor3ub(255, 255, 255); // the vertices are drawn with white colour
    glVertex3f(pgoiptr->x, pgoiptr->y, pgoiptr->z);
    glVertex3f(perdiptr->x, perdiptr->y, perdiptr->z);
    glVertex3f(pbeheptr->x, pbeheptr->y, pbeheptr->z);
    glEnd();

    // the following code is not taken into account.. But it should be!

    // TODO draw the lines of the polygon. Ertzak marraztu
    //  1-2 ertza
    draw_edge(optr, indg, indb, atzeaurpegiada);
    // 1-3 ertza
    draw_edge(optr, indg, inde, atzeaurpegiada);
    // 2-3 ertza
    draw_edge(optr, inde, indb, atzeaurpegiada);
    // if the user wants only the limits of the triangles (only edges) the work is done.
    if (lineak == 1)
        return;

    // In othe case the triangle must be filled,
    // fill the triangle drawing horizontall segments
    // Segmentuz-segmentu marratzuko dut:

    // TODO draw the segments of the triangle in two parts: the upper segments and lower segments.

    // TODO draw upper segments: from upper vertex until midle vertex-
    //      goiko erpinaren eta erdiko erpinaren arteko segmentu horizontalak marraztu

    // TODO draw lower segments: from the midle to the lower vertex.
    //      erdiko erpinaren eta beheko erpinaren arteko segmentu horizontalak marraztu

    return;
}

void dibujar_poligono(object3d *optr, int ti)
{
    int i, ind0, ind1, ind2;
    int atzeaurpegiada;
    double *Nkam;

    if (ti >= optr->num_faces)
        return;
    // lehenengo hiru erpinekin kalkulatuko dut ikusgaitasuna.
    ind0 = optr->face_table[ti].vertex_ind_table[0];
    ind1 = optr->face_table[ti].vertex_ind_table[1];
    ind2 = optr->face_table[ti].vertex_ind_table[2];

    // TODO erabaki marraztu behar den ala ez: ikuste bolumenetik kanpokoak ez marraztu

    atzeaurpegiada = 0;
    // TODO atze-aurpegia?
    // atzeaurpegiada = ...
    if ((!atzearpegiakmarraztu) && atzeaurpegiada)
    {
        // Back culling...
        return;
    }
    if (optr->texturaduna == 0)
    {
        // TODO erpin bakoitzaren kolorea kalkulatu: argien, kameraren eta objektuaren orientazioaren arabera.
        argien_kalkulua_egin(optr, ti);
    }
    // honaino iritsi bada bere triangelu guztiak marraztu behar ditut
    for (i = 1; i < (optr->face_table[ti].num_vertices - 1); i++) // triangeluka marraztu: 4 erpinekin bi triangelu, bostekin 3...
        dibujar_triangulo(optr, ti, i, atzeaurpegiada);
}

static void marraztu(void)
{
    float u, v;
    int i, j;
    object3d *auxptr;
    double Fokudir[3];

    // marrazteko objektuak behar dira
    // no se puede dibujar sin objetos
    if (foptr == 0)
        return;

    // clear viewport...
    if (objektuak == 1)
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    else
    {
        if (denak == 0)
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    }
    // TODO Ikuslearen edo kameraren erreferentzia-sistemara pasatzen duen matrizea lortu

    // TODO argiek kameraren ikuspegian duten informazioa eguneratu (bai kokapenak, bai direkzioak)

    if (objektuak == 1)
    {
        if (denak == 1)
        {
            // printf("objektuak marraztera\n");
            for (auxptr = foptr; auxptr != 0; auxptr = auxptr->hptr)
            {
                // TODO objektua kamerak nola ikusten duen adierazi objektuaren egituan bertan.
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
            // TODO objektua kamerak nola ikusten duen adierazi objektuaren egituan bertan.
            kam_ikuspegia_lortu(sel_ptr);
            for (i = 0; i < sel_ptr->num_faces; i++)
            {
                dibujar_poligono(sel_ptr, i);
            }
        }
    }
    else
    {
        // TODO objektua kamerak nola ikusten duen adierazi objektuaren egituan bertan.
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
        /*
         //printf("objektuaren matrizea...\n");
         optr->mptr = (mlist *)malloc(sizeof(mlist));
         for (i=0; i<16; i++) optr->mptr->m[i] =0;
         optr->mptr->m[0] = 1.0;
         optr->mptr->m[5] = 1.0;
         optr->mptr->m[10] = 1.0;
         optr->mptr->m[15] = 1.0;
         optr->mptr->hptr = 0;
         */
        // printf("objektu edo kamera zerrendara doa informazioa...\n");
        optr->hptr = *foptrptr;
        *foptrptr = optr;
        // printf("normalak kalkulatzera\n");
        // for (i = 0; i< optr->num_vertices; i++) printf("%lf %lf %lf\n", optr->vertex_table[i].coord.x,optr->vertex_table[i].coord.y,optr->vertex_table[i].coord.z);
        obj_normalak_kalkulatu(optr);
        sel_ptr = optr;
        if (optr->texturaduna && (bufferra == 0))
        {
            // we put the information of the texture in the buffer pointed by bufferra. The dimensions of the texture are loaded into dimx and dimy
            retval = load_ppm("testura.ppm", &bufferra, &dimx, &dimy);
            if (retval != 1)
            {
                printf("Ez dago testuraren fitxategia (testura.ppm)\n");
                optr->texturaduna = 0;
                // exit(-1);
            }
        }
    }
    // printf("datuak irakurrita\n");
}

void x_aldaketa(int dir)
{

    if (kamera == 0) // objektua aldatzen ari naiz
    {
        if (aldaketa == 'r')
        {
            // rotate cos(5) = 0.99619469809174;  // cos(5)
        }
        else
        {
            // traslate: 0.02?
        }
    }
    else if (kamera == 1) // kamera aldatzen ari naiz
    {
        if (ald_lokala == 1) // hegaldi moduan ezker/eskuin begiratu (y-rekiko biraketa)
        {
            // TODO hegan egin
        }
        else // analisi moduan
        {
            if (sel_ptr != 0)
            {
                // aukeratutako objektua analizatu eskuinerago edo ezkerreragotik (biratuz!)
            }
        }
    }
    else // argiak aldatzen
    {
        // eguzkia biratu edo bonbila mugitu
    }
}

void y_aldaketa(int dir)
{

    if (kamera == 0) // objektua aldatzen ari naiz
    {
        if (aldaketa == 'r')
        {
            // rotate cos(5) = 0.99619469809174;  // cos(5)
        }
        else
        {
            // traslate: 0.02?
        }
    }
    else if (kamera == 1) // kamera aldatzen ari naiz
    {
        if (ald_lokala == 1) // hegaldi moduan ezker/eskuin begiratu (y-rekiko biraketa)
        {
            // TODO hegan egin
        }
        else // analisi moduan
        {
            if (sel_ptr != 0)
            {
                // aukeratutako objektua analizatu goragotik edo beheragotik (biratuz!)
            }
        }
    }
    else // argiak aldatzen
    {
        // eguzkia biratu edo bonbila mugitu
    }
}

void z_aldaketa(int dir)
{
    if (kamera == 0) // objektuari aldaketa
    {
        if (aldaketa == 'r')
        {
            // rotate cos(5) = 0.99619469809174;  // cos(5)
        }
        else
        {
            // translate
        }
    }
    else // kamerari aldaketa
        if (kamera == 1)
        {
            // hegaldi moduan beti aurrera edo atzera mugitu kamera.
            // analisi moduan traslazioa egin nahi bada objektura gerturatu (pasa gabe!! distantzia kontrolatu) edo urrutiratu
            // analisi moduan biraketa (roll)
        }
        else // argiak aldatzen
        {
            // bonbilla mugitu munduan edo eguzkia biratu?
        }
}

void undo()
{
}

void kamera_objektuari_begira()
{
}

void print_egoera()
{
    if (kamera == 0)
    {
        if (ald_lokala == 1)
            printf("\nobjektua aldatzen ari zara, (aldaketa lokala)\n");
        else
            printf("\nobjektua aldatzen ari zara, (aldaketa globala)\n");
    }
    if (kamera == 1)
    {
        if (ald_lokala == 1)
            printf("\nkamera aldatzen ari zara hegaldi moduan\n");
        else
            printf("\nkamera aldatzen ari zara analisi-moduan\n");
    }
    if (kamera == 2)
    {
        printf("\nargiak aldatzen ari zara\n");
    }
    if (aldaketa == 't')
        printf("Traslazioa dago aktibatuta\n");
    else
        printf("Biraketak daude aktibatuta\n");
    if (objektuaren_ikuspegia)
        printf("objektuaren ikuspuntua erakusten ari zara (`C` sakatu kamerarenera pasatzeko)\n");
}

// This function will be called whenever the user pushes one key
static void teklatua(unsigned char key, int x, int y)
{
    int retval;
    int i;
    FILE *obj_file;

    switch (key)
    {
    case 13:
        if (foptr != 0) // objekturik ez badago ezer ez du egin behar
                        // si no hay objeto que no haga nada
        {
            indexx++; // azkena bada lehenengoa bihurtu
                      // pero si es el último? hay que controlarlo!
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
        if (kamera == 0) // objektua aldatzen ari naiz.
        {
            if (objektuaren_ikuspegia == 1) // objektuaren ikuspegian banago argiak aldatzera pasa naiteke, ez kamera aldatzera.
            {
                kamera = 2;
                printf("argiak aldatzera zoaz (objektuaren ikuspegian zaude)\n");
            }
            else
            {
                kamera = 1;
                ald_lokala = 1; // hegaldi moduan jarriko naiz
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
                ald_lokala = 1; // hegaldi moduan jarriko naiz
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
        if (objektuaren_ikuspegia == 0) // objektuaren ikuspegian aldaketa beti lokala izango da.
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
    case 's': // argiztapen suabizatua
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
        // Eguzkia piztu/itzali
        break;
    case '2':
        // Bonbilla piztu/itzali
        break;
    case '3':
        // objektuaren fokua piztu/itzali
        break;
    case '4':
        // kameraren fokua piztu/itzali
        break;
    case '+':
        if (kamera == 2)
        { // Fokuen irekiera handitu
        }
        else
        {
            if (kamera == 1)
            { // kameraren ikuste-bolumena handitu
            }
        }
        break;
    case '-':
        if (kamera == 2)
        { // Fokuen irekiera txikitu
        }
        else
        {
            if (kamera == 1)
            { // kameraren ikuste-bolumena txikitu
            }
        }
        break;
    case 'f':
        /*Ask for file*/
        printf("idatzi fitxategi izena\n");
        scanf("%s", &(fitxiz[0]));
        read_from_file(fitxiz, &foptr);
        indexx = 0;
        if ((ald_lokala == 0) && (kamera == 1) && (sel_ptr != 0))
            kamera_objektuari_begira();
        break;
    case 9:             /* <TAB> */
        if (foptr != 0) // objekturik gabe ez du ezer egin behar
                        // si no hay objeto no hace nada
        {
            sel_ptr = sel_ptr->hptr;
            /*The selection is circular, thus if we move out of the list we go back to the first element*/
            if (sel_ptr == 0)
                sel_ptr = foptr;
            indexx = 0; // the selected polygon is the first one
            if ((ald_lokala == 0) && (kamera == 1))
            {
                // kamera objektuari begira jarri behar da!!
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
    // The screen must be drawn to show the new triangle
    glutPostRedisplay();
}

void viewportberria(int zabal, int garai)
{
    if (zabal < garai)
        dimentsioa = zabal;
    else
        dimentsioa = garai;
    glViewport(0, 0, dimentsioa, dimentsioa);
    printf("linea kopuru berria = %d\n", dimentsioa);
}

int main(int argc, char **argv)
{
    int retval, i;

    printf(" Triangeluak: barneko puntuak eta testura\n Triángulos con puntos internos y textura \n");
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
    glClearDepth(0.0); // Handiena marraztu
    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    // glOrtho(-1.0, 1.0, -1.0, 1.0, 1.0, -1.0);
    // glMatrixMode(GL_MODELVIEW);
    denak = 0;
    lineak = 0;
    objektuak = 0;
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
    // TODO kamera hasieratu kamera (0,0,2.5) kokapenean dago hasieran

    // TODO Argiak hasieratu

    if (argc > 1)
        read_from_file(argv[1], &foptr);
    else
    {
        // TODO erase mesages and exit code!!
        printf("Aldatu kode zati hau!!!! edo exekutatu objektua daukan fitxategi-izen batekin\n");
        printf("    cambia el código!!!! ó ejecútalo con un objeto\n");
        exit(0);
        // TODO load some object[s] by default
        /*
        read_from_file("abioia-1+1.obj",&foptr);
        if (sel_ptr != 0)
            { //sel_ptr->mptr->m[3] = -1.0;
            }
        read_from_file("abioia-1+1.obj",&foptr);
        if (sel_ptr != 0)
            { //sel_ptr->mptr->m[3] = 1.0;
            }
        read_from_file("abioia-1+1.obj",&foptr);
        if (sel_ptr != 0)
            {
            //sel_ptr->mptr->m[7] = -0.4;
            //sel_ptr->mptr->m[11] = 0.5;
            }
        read_from_file("abioia-1+1.obj",&foptr);
        if (sel_ptr != 0)
            { //sel_ptr->mptr->m[11] = -0.7;
            }
        read_from_file("abioia-1+1.obj",&foptr);
        if (sel_ptr != 0)
            { //sel_ptr->mptr->m[7] = 0.7;
            }
        */
        // read_from_file("z-1+1.obj",&foptr);
        // read_from_file("abioia-1+1.obj",&foptr);
        // read_from_file("triangles-1+1.obj",&foptr);
    }
    print_egoera();
    glutMainLoop();

    return 0;
}
