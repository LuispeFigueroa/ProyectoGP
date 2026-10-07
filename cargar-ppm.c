#include <stdio.h>
#include <unistd.h>
#include <malloc.h>

int load_ppm(char *file, unsigned char **bufferptr, int *dimxptr, int * dimyptr)
{
 char line[40];
 int luz,zbkia;
 FILE *obj_file;
 int fd;
 
 if ((obj_file = fopen(file, "r")) == NULL) 
        {
        *dimxptr= 0;
        *dimyptr=0;
        *bufferptr = (unsigned char *)0;
        return(-1);
        }
    /* leer el formato del archivo */
luz =fscanf(obj_file, "%[^\n]\n", line);
if ( luz > 1) 
    {
    if ((line[0]== 'P')&&(line[1]=='6'))
        printf("formato correcto\n");
      else
        {
        *dimxptr= 0;
        *dimyptr=0;
        *bufferptr = (unsigned char*)0;
        printf("el formato debe ser de tipo P6\n");
        return(-1);
        }
    }
    /* leer el tamaño del archivo */
luz =fscanf(obj_file, "%[^\n]\n", line);
if (luz>0) 
    {
    luz = sscanf(line,"%d %d",dimxptr,dimyptr);
    if (luz == 2)
        printf("dimensiones leídas: %d,%d\n",*dimxptr,*dimyptr);
      else
        {
        *dimxptr= 0;
        *dimyptr=0;
        *bufferptr = (unsigned char*)0;
        printf("problemas al leer las dimensiones\n");
        return(-1);
        }
    }
    /* leer la expresión de color del archivo */
luz =fscanf(obj_file, "%[^\n]\n", line);
if (luz>0) 
    {
    luz = sscanf(line,"%d",&zbkia);
    if (luz == 1)
        printf("valor máximo de color leído: %d\n",zbkia);
      else
        {
        *dimxptr= 0;
        *dimyptr=0;
        *bufferptr = (unsigned char*)0;
        printf("problemas al leer la expresión de color\n");
        return(-1);
        }
    }
luz=(*dimxptr)*(*dimyptr)*3;
*bufferptr = (unsigned char *)malloc(luz);
zbkia=fread(*bufferptr,1,luz,obj_file);
/*
fd = fileno(obj_file);
zbkia = read(fd, bufferptr, luz);
*/
if (zbkia != luz) 
        {
	*dimxptr= 0;
        *dimyptr=0;
        free(*bufferptr);
        *bufferptr = (void*)0;
        printf("error al llenar el buffer...zbkia = %d, luz =%d\n",zbkia,luz);
        return(-1);
        }
      else
        {
        printf("buffer leído correctamente\n");
        return(1);
        }
}
/*
int main(int argc, char *argv[])
{
int dimx, dimy;
void * buferra;

load_ppm("joseba-eskiatzen.ppm", &buferra, &dimx, &dimy);
printf("¡¡¡imagen cargada!!! tiene dimensiones %d, %d\n",dimx,dimy);
}
*/
