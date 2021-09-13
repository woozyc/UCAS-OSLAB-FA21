#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FILE "./image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."

#define OSSIZE_LOC_BASE 0x1fb


/* structure to store command line options */
static struct {
    int vm;
    int extended;
} options;

/* prototypes of local functions */
static void create_image(int nfiles, char *files[]);
static void error(char *fmt, ...);
static void read_ehdr(Elf64_Ehdr * ehdr, FILE * fp);
static void read_phdr(Elf64_Phdr * phdr, FILE * fp, int ph,
                      Elf64_Ehdr ehdr);
static void write_segment(Elf64_Ehdr ehdr, Elf64_Phdr phdr, FILE * fp,
                          FILE * img, int *nbytes, int *count);
static void pad_section(int *nbytes, FILE * img);
static void write_os_size(int nbytes, FILE * img, int count);
static void write_os_num(FILE * img, int count);

int main(int argc, char **argv)
{
    char *progname = argv[0];

    /* process command line options */
    options.vm = 0;
    options.extended = 0;
    while ((argc > 1) && (argv[1][0] == '-') && (argv[1][1] == '-')) {
        char *option = &argv[1][2];

        if (strcmp(option, "vm") == 0) {
            options.vm = 1;
        } else if (strcmp(option, "extended") == 0) {
            options.extended = 1;
        } else {
            error("%s: invalid option\nusage: %s %s\n", progname,
                  progname, ARGS);
        }
        argc--;
        argv++;
    }
    if (options.vm == 1) {
        error("%s: option --vm not implemented\n", progname);
    }
    if (argc < 3) {
        /* at least 3 args (createimage bootblock kernel) */
        error("usage: %s %s\n", progname, ARGS);
    }
    create_image(argc - 1, argv + 1);
    return 0;
}

static void create_image(int nfiles, char *files[])
{
    int ph, nbytes, count = 0;
    //nbytes: only kernel bytes, count: kernel number
    FILE *fp, *img;
    Elf64_Ehdr ehdr;
    Elf64_Phdr phdr;

    /* open the image file */
    img = fopen(IMAGE_FILE, "wb+");
    if(!img){
    	printf("Error! Cannot open file %s\n", IMAGE_FILE);
    	return ;
    }

    /* for each input file */
    while (nfiles-- > 0) {
    	nbytes = 0;

        /* open input file */
        fp = fopen(*files, "rb");
        if(!fp){
        	printf("Error! Connot open file %s\n", *files);
        	return ;
        }

        /* read ELF header */
        read_ehdr(&ehdr, fp);
        printf("0x%04lx: %s\n", ehdr.e_entry, *files);

        /* for each program header */
        for (ph = 0; ph < ehdr.e_phnum; ph++) {

            /* read program header */
            read_phdr(&phdr, fp, ph, ehdr);
            printf("\tsegment %d\n", ph);
            printf("\t\toffset 0x%04lx\t\tvaddr 0x%04lx\n", phdr.p_offset, phdr.p_vaddr);
            printf("\t\tfilesz 0x%04lx\t\tmemsz 0x%04lx\n", phdr.p_filesz, phdr.p_memsz);

            /* write segment to the image */
            write_segment(ehdr, phdr, fp, img, &nbytes, &count);
            if(!ph)
	            printf("\t\twriting 0x%04x bytes\n", nbytes);
        }
        
        pad_section(&nbytes, img);
    	printf("\tpadding up to 0x%04lx\n", ftell(img));
    	write_os_size(nbytes, img, count);
    	
        fclose(fp);
        files++, count++;
    }
    write_os_num(img, --count);
    fclose(img);
}

static void read_ehdr(Elf64_Ehdr * ehdr, FILE * fp)
{
    if(!fp)
    	return ;
    fread(ehdr, sizeof(Elf64_Ehdr), 1, fp);
}

static void read_phdr(Elf64_Phdr * phdr, FILE * fp, int ph,
                      Elf64_Ehdr ehdr)
{
    if(!fp)
    	return ;
    fseek(fp, ph * sizeof(Elf64_Phdr) + ehdr.e_phoff, SEEK_SET);
    fread(phdr, sizeof(Elf64_Phdr), 1, fp);
}

static void write_segment(Elf64_Ehdr ehdr, Elf64_Phdr phdr, FILE * fp,
                          FILE * img, int *nbytes, int *count)
{
	char pad[512] = "";	
	
	if(!img || !fp)
		return ;
	
	//read segment into *segment
    char *segment;
    segment = malloc(phdr.p_filesz);
    if(!segment){
    	printf("Error! Malloc failed\n");
    	return ;
    }
    fseek(fp, phdr.p_offset, SEEK_SET);
    fread(segment, phdr.p_filesz, 1, fp);
    
    //write segment into img
    fwrite(segment, phdr.p_filesz, 1, img);
    
    //pad to p_memsz
    fwrite(pad, phdr.p_memsz - phdr.p_filesz, 1, img);
    *nbytes += phdr.p_memsz;
}

static void pad_section(int *nbytes, FILE * img){//pad the rest of section
	char pad[512] = "";
	int diff = (*nbytes) % 512;
	
	if(!img)
		return ;
	if(diff){
		diff = 512 - diff;
		fwrite(pad, diff, 1, img);
		*nbytes += diff;
	}
	
}

static void write_os_size(int nbytes, FILE * img, int count)
{
	//save pointer
	long pointer = ftell(img);
	if(!img)
		return ;
	int sec = nbytes / 512;
	char sections[2] = {sec / 256 , sec % 256};//half word
    if(count){//not bootloader
    	fseek(img, OSSIZE_LOC_BASE - 4 * count, SEEK_SET);//0x5e0001fc - 4, - 8...
    	fwrite(sections, 2L, 1, img);
    }
    if(count)
	    printf("\tkernel %2d size: %d\n", count, sec);
	else
		printf("\tbootblock size: %d\n", sec);
    
	//restore pointer
	fseek(img, pointer, SEEK_SET);
}

static void write_os_num(FILE * img, int count){
	if(!img)
		return ;
	
	char num[2] = {count / 256 , count % 256};//half word
    fseek(img, OSSIZE_LOC_BASE, SEEK_SET);//0x502001fc
   	fwrite(num, 2L, 1, img);
   	printf("total kernel number: %d\n", count);
}


/* print an error message and exit */
static void error(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    if (errno != 0) {
        perror(NULL);
    }
    exit(EXIT_FAILURE);
}
