#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
//#include "definitions.h"
#include "obj.h"

#define MAXLINE 200

/*
 * Función auxiliar para procesar cada línea del archivo
 */
static int sreadint(char * lerroa, int * zenbakiak) {
    char *s = lerroa;
    int i, zbk, kont = 0;

    while (sscanf(s, " %d%n", &zbk, &i) > 0) {
        s += i;
        zenbakiak[kont++] = zbk;
    }
    return (kont);
}

static int sreadint2(char * lerroa, int * zenbakiak) {
    char *s = lerroa;
    int i, zbk, kont = 0;

    while (sscanf(s, " %d%n", &zbk, &i) > 0) {
        s += i;
	while ((*s != ' ')&&(*s !='\0')) s++;  // saltar la información del vector normal
        zenbakiak[kont++] = zbk;
    }
    //printf("%d numbers in the line\n",kont);
    return (kont);
}
/**
 * @brief Función para leer archivos wavefront (*.obj)
 * @param file_name Ruta del archivo a leer
 * @param object_ptr Puntero a la estructura de tipo object3d donde se guardarán los datos
 * @return Resultado de la lectura: 0=Lectura correcta, 1=Archivo no encontrado, 2=Archivo no válido, 3=Archivo vacío
 */
int read_wavefront(char * file_name, object3d * object_ptr) {
    vertex *vertex_table;
    face *face_table;
    int num_vertices = -1, num_faces = -1, count_vertices = 0, count_faces = 0, count_textures =0;
    FILE *obj_file;
    char line[MAXLINE], line_1[MAXLINE], aux[45];
    int k, t;
    int i, j;
    int r, g, b;
    int texturaduna, koloreduna;
    int values[MAXLINE];

    koloreduna = 0;
    /*
     * La función lee el archivo dos veces. En la primera lectura se obtiene
     * el número de vértices y de caras. Después, se reserva memoria para
     * cada uno de ellos y en la segunda lectura se lee y se carga la
     * información real. Finalmente, se crea la estructura del objeto
     */
    if ((obj_file = fopen(file_name, "r")) == NULL) return (1);
    while (fscanf(obj_file, "\n%[^\n]", line) > 0) 
        {
        i = 0;
        while (line[i] == ' ') i++;
        if ((line[0] == '#') && ((int) strlen(line) > 5)) 
            {
            i += 2;
            j = 0;
            // es posible una línea de la forma "# number vertices" donde "number" es un número
            // es posible una línea de la forma "# number elements" donde "number" es un número
            // es posible una línea de la forma "# color r g b" donde "r", "g" y "b" son números <256
            while (line[i] != ' ') line_1[j++] = line[i++];
            i++;
            line_1[j] = '\0';
            j = 0;
            if ((strcmp(line_1, "color") == 0) || (strcmp(line_1, "colour") == 0))
                {
                k = sscanf(line + i, "%d%d%d",&r,&g,&b);
                if (k ==3) 
                    {
                    koloreduna = 1;
                    object_ptr->rgb.r = r;
                    object_ptr->rgb.g = g;
                    object_ptr->rgb.b = b;
                    }
                }
              else
                {
                while ((line[i] != ' ') && (line[i] != '\0'))
                    aux[j++] = line[i++];
                aux[j] = 0;
                if (strcmp(aux, "vertices") == 0)  num_vertices = atoi(line_1);
                if (strncmp(aux, "elements", 7) == 0) num_faces = atoi(line_1);
                }
            } 
          else 
            {
            if (strlen(line) > 6)
                {
                if (line[i] == 'f' && line[i + 1] == ' ')
                    count_faces++;
                else 
                    if (line[i] == 'v' && line[i + 1] == ' ')
                        count_vertices++;
                    else
                         if (line[i] == 't' && line[i + 1] == ' ')
                             count_textures ++;
                     
                }
            }
        }
    fclose(obj_file);
    //printf("1 pasada: num vert = %d (%d), num faces = %d(%d) \n",num_vertices,count_vertices,num_faces,count_faces);
    if ((num_vertices != -1 && num_vertices != count_vertices) || (num_faces != -1 && num_faces != count_faces)) {
        printf("WARNING: full file format: (%s)\n", file_name);
        //return (2);
    }
    if (num_vertices == 0 || count_vertices == 0) {
        printf("No vertex found: (%s)\n", file_name);
        return (3);
    }
    if (num_faces == 0 || count_faces == 0) {
        printf("No faces found: (%s)\n", file_name);
        return (3);
    }

    num_vertices = count_vertices;
    num_faces = count_faces;
    if (count_vertices == count_textures) texturaduna = 1;
        else 
          {
          if (koloreduna == 0) // ¡el objeto no tiene textura ni color!
              {
              // asignar color al objeto (blanco)
              object_ptr->rgb.r = 255;
              object_ptr->rgb.g = 255;
              object_ptr->rgb.b = 255;
              }
          texturaduna = 0;
          }

    vertex_table = (vertex *) malloc(num_vertices * sizeof (vertex));
    face_table = (face *) malloc(num_faces * sizeof (face));

    obj_file = fopen(file_name, "r");
    k = 0;  // número de vértice
    j = 0;  // número de cara
    t = 0;  // número de textura

    for (i = 0; i < num_vertices; i++)
        vertex_table[i].num_faces = 0;

    while (fscanf(obj_file, "\n%[^\n]", line) > 0) {
        switch (line[0]) {
            case 'v':
            if (line[1] == ' ')  // vn no interesa
		        {
                sscanf(line + 2, "%lf%lf%lf", &(vertex_table[k].coord.x),
                        &(vertex_table[k].coord.y), &(vertex_table[k].coord.z));
                k++;
		        }
               break;
            case 't':
            if (texturaduna && (line[1] == ' '))  // línea de la forma "t u v"
		        {
                sscanf(line + 2, "%lf%lf", &(vertex_table[t].u),
                        &(vertex_table[t].v));
                t++;
		        }
               break;

            case 'f':
	        if (line[1] == ' ') // fn no interesa
                {
                for (i = 2; i <= (int) strlen(line); i++)
                    line_1[i - 2] = line[i];
		        line_1[i-2] = '\0';
                face_table[j].num_vertices = sreadint2(line_1, values);
//printf("f %d vertices\n",face_table[j].num_vertices);
                face_table[j].vertex_ind_table = (int *) malloc(face_table[j].num_vertices * sizeof (int));
                for (i = 0; i < face_table[j].num_vertices; i++) {
                    face_table[j].vertex_ind_table[i] = values[i] - 1;
//printf(" %d ",values[i] - 1);
                    vertex_table[face_table[j].vertex_ind_table[i]].num_faces++;
                    }
//printf("\n");
                j++;
                }
              break;
        }
    }

    fclose(obj_file);

    //printf("2 pasada\n");

    /*
     * La información leída se introduce en la estructura */
    object_ptr->vertex_table = vertex_table;
    object_ptr->face_table = face_table;
    object_ptr->num_vertices = num_vertices;
    object_ptr->num_faces = num_faces;
    object_ptr->texturaduna = texturaduna;


    /*
     * Se obtienen las coordenadas máximas y mínimas **/
    object_ptr->max.x = object_ptr->vertex_table[0].coord.x;
    object_ptr->max.y = object_ptr->vertex_table[0].coord.y;
    object_ptr->max.z = object_ptr->vertex_table[0].coord.z;
    object_ptr->min.x = object_ptr->vertex_table[0].coord.x;
    object_ptr->min.y = object_ptr->vertex_table[0].coord.y;
    object_ptr->min.z = object_ptr->vertex_table[0].coord.z;

    for (i = 1; i < object_ptr->num_vertices; i++)
    {
        if (object_ptr->vertex_table[i].coord.x < object_ptr->min.x)
            object_ptr->min.x = object_ptr->vertex_table[i].coord.x;

        if (object_ptr->vertex_table[i].coord.y < object_ptr->min.y)
            object_ptr->min.y = object_ptr->vertex_table[i].coord.y;

        if (object_ptr->vertex_table[i].coord.z < object_ptr->min.z)
            object_ptr->min.z = object_ptr->vertex_table[i].coord.z;

        if (object_ptr->vertex_table[i].coord.x > object_ptr->max.x)
            object_ptr->max.x = object_ptr->vertex_table[i].coord.x;

        if (object_ptr->vertex_table[i].coord.y > object_ptr->max.y)
            object_ptr->max.y = object_ptr->vertex_table[i].coord.y;

        if (object_ptr->vertex_table[i].coord.z > object_ptr->max.z)
            object_ptr->max.z = object_ptr->vertex_table[i].coord.z;

    }
    return (0);
}
