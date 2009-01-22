#include <stdio.h>
#include <vector>

//#define BENCHMARK

class DocData {
	public:
	float fileSize ;
	float deltaSize ;
	float origdeltaSize ;
#ifdef BENCHMARK
	float phase1 ;
	float phase2 ;
	float phase3 ;
#endif
	int fileNB ;
	} ;

vector<class DocData> stats ;

int main(int argc, char **argv) {

	if (argc<2) {
		printf("exec example1.xml\n");
		exit(0);
		}
		
#ifdef BENCHMARK
	printf("opening timeFile\n");
	FILE *timeFile = fopen("timefile.txt", "rb");
	if (timeFile==NULL) exit(0);
#endif

	FILE *dsPerfs = fopen("dsperfs.dat", "wb");
	if (dsPerfs==NULL) exit(0);
#ifdef BENCHMARK
	FILE *ph1fs = fopen("ph1fs.dat", "wb" );
	FILE *ph2fs = fopen("ph2fs.dat", "wb" );
	FILE *ph3fs = fopen("ph3fs.dat", "wb" );
#endif
	FILE *dsPods = fopen("dsPods.dat", "wb" );
	
	int fileNB = 0 ;
#ifdef BENCHMARK
	while(!feof(timeFile)) {
		int mphase1, mphase2, mphase3 ;
		fscanf(timeFile, "phase1: %5dms, phase2: %5dms, phase3: %5dms\n", &mphase1, &mphase2, &mphase3);
#else
	while(1) {
#endif		

		class DocData data ;

		char filename[100] ;
		if (fileNB==0) sprintf(filename,"%s",argv[1]);
		else sprintf(filename,"%s.result%03d.xml", argv[1], fileNB-1);
		printf("file=%s\n", filename);
		FILE *file = fopen(filename, "rb");
		if (file==NULL) exit(0);
		(void)fseek(file, 0, SEEK_END);
		data.fileSize = ftell( file );
		fclose(file);
		
		char deltafilename[100] ;
		sprintf(deltafilename, "%s.forwardDelta.xml", filename);
		printf("file=%s\n", deltafilename);
		file = fopen(deltafilename, "rb");
		if (file==NULL) exit(0);
		(void)fseek(file, 0, SEEK_END);
		data.deltaSize = ftell( file );
		fclose(file);

		char origdeltafilename[100] ;
		sprintf(origdeltafilename, "%s.script%03d.delta.xml", argv[1], fileNB);
		printf("origDeltaFile=%s\n", origdeltafilename);
		file = fopen(origdeltafilename, "rb");
		if (file==NULL) exit(0);
		(void)fseek(file, 0, SEEK_END);
		data.origdeltaSize = ftell( file );
		fclose(file);
		
#ifdef BENCHMARK
		data.phase1 = mphase1 ;
		data.phase2 = mphase2 ;
		data.phase3 = mphase3 ;
#endif
		data.fileNB = ++fileNB ;
#ifdef BENCHMARK
		printf("File #%3d, fSize=%6d, deltaSize=%6d(orig=%6d), phase1=%f, phase2=%f, phase3=%f\n", (int)data.fileNB, (int)data.fileSize, (int)data.deltaSize,	(int)data.origdeltaSize, data.phase1, data.phase2, data.phase3);
#else
		printf("File #%3d, fSize=%6d, deltaSize=%6d(orig=%6d)\n", (int)data.fileNB, (int)data.fileSize, (int)data.deltaSize,	(int)data.origdeltaSize);
#endif		
		fprintf(dsPerfs,"%f %f\n", (float)data.fileSize, (float)data.deltaSize);
#ifdef BENCHMARK
		fprintf(ph1fs,  "%f %f\n", (float)data.fileSize, (float)data.phase1);
//		fprintf(ph2fs,  "%f %f\n", (float)data.fileSize, (float)data.phase1+data.phase2);
//		fprintf(ph3fs,  "%f %f\n", (float)data.fileSize, (float)data.phase1+data.phase2+data.phase3);
		fprintf(ph2fs,  "%f %f\n", (float)data.fileSize, (float)data.phase2);
		fprintf(ph3fs,  "%f %f\n", (float)data.fileSize, (float)data.phase3);
#endif
		fprintf(dsPods,  "%f %f\n", (float)data.origdeltaSize, (float)data.deltaSize);
		}
	
#ifdef BENCHMARK
	fclose(timeFile);
	fclose(ph1fs);
	fclose(ph2fs);
	fclose(ph3fs);
#endif
	fclose(dsPerfs);
	fclose(dsPods);	
	printf("finished.\n");
	}
