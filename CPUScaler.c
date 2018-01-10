#include <stdio.h>
#include <jni.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include<inttypes.h>
#include "CPUScaler.h"
#include "arch_spec.h"
#include "msr.h"

rapl_msr_unit rapl_unit;
rapl_msr_parameter *parameters;
char *ener_info;
/*global variable*/
int *fd;

void copy_to_string(char *ener_info, char uncore_buffer[60], int uncore_num, char cpu_buffer[60], int cpu_num, char package_buffer[60], int package_num, int i, int *offset) {
	memcpy(ener_info + *offset, &uncore_buffer, uncore_num);
	//split sigh
	ener_info[*offset + uncore_num] = '#';
	memcpy(ener_info + *offset + uncore_num + 1, &cpu_buffer, cpu_num);
	ener_info[*offset + uncore_num + cpu_num + 1] = '#';
	if(i < num_pkg - 1) {
		memcpy(ener_info + *offset + uncore_num + cpu_num + 2, &package_buffer, package_num);
		offset += uncore_num + cpu_num + package_num + 2;
		if(num_pkg > 1) {
			ener_info[*offset] = '@';
			offset++;
		}
	} else {
		memcpy(ener_info + *offset + uncore_num + cpu_num + 2, &package_buffer, package_num + 1);
	}

}


JNIEXPORT jint JNICALL Java_EnergyCheckUtils_ProfileInit(JNIEnv *env, jclass jcls) {
	jintArray result;
	int i;
	char msr_filename[BUFSIZ];

	get_cpu_model();	
	getSocketNum();

	jint wraparound_energy;

	/*only two domains are supported for parameters check*/
	parameters = (rapl_msr_parameter *)malloc(2 * sizeof(rapl_msr_parameter));
	fd = (int *) malloc(num_pkg * sizeof(int));

	for(i = 0; i < num_pkg; i++) {
		if(i > 0) {
			core += num_pkg_thread / 2; 	//measure the first core of each package
		}
		sprintf(msr_filename, "/dev/cpu/%d/msr", core);
		fd[i] = open(msr_filename, O_RDWR);
	}

	uint64_t unit_info= read_msr(fd[0], MSR_RAPL_POWER_UNIT);
	//printf("open core: %d\n", core);
	get_msr_unit(&rapl_unit, unit_info);
	get_wraparound_energy(rapl_unit.energy);
	wraparound_energy = (int)WRAPAROUND_VALUE;

	return wraparound_energy;
}

JNIEXPORT jint JNICALL Java_EnergyCheckUtils_GetSocketNum(JNIEnv *env, jclass jcls) {
	return (jint)getSocketNum();
}

#define MSR_DRAM_ENERGY_UNIT 0.000015

void
initialize_energy_info(char gpu_buffer[num_pkg][60], char dram_buffer[num_pkg][60], char cpu_buffer[num_pkg][60], char package_buffer[num_pkg][60]) {

	double package[num_pkg];
	double pp0[num_pkg];
	double pp1[num_pkg];
	double dram[num_pkg];
	double result = 0.0;
	int info_size = 0;
	int i = 0;
	for (; i < num_pkg; i++) {

		result = read_msr(fd[i], MSR_PKG_ENERGY_STATUS);	//First 32 bits so don't need shift bits.
		package[i] = (double) result * rapl_unit.energy;

		result = read_msr(fd[i], MSR_PP0_ENERGY_STATUS);
		pp0[i] = (double) result * rapl_unit.energy;

		//printf("package energy: %f\n", package[i]);

		sprintf(package_buffer[i], "%f", package[i]);
		sprintf(cpu_buffer[i], "%f", pp0[i]);
		
		//allocate space for string
		//printf("%" PRIu32 "\n", cpu_model);
		switch(cpu_model) {
			case SANDYBRIDGE_EP:
			case HASWELL1:
			case HASWELL2:
			case HASWELL3:
			case HASWELL_EP:
			case SKYLAKE1:
			case SKYLAKE2:
			case BROADWELL:
			case BROADWELL2:
	
				result = read_msr(fd[i],MSR_DRAM_ENERGY_STATUS);
				if (cpu_model == BROADWELL || cpu_model == BROADWELL2) {
					dram[i] =(double)result*MSR_DRAM_ENERGY_UNIT;
				} else {
					dram[i] =(double)result*rapl_unit.energy;
				}

				sprintf(dram_buffer[i], "%f", dram[i]);

				info_size += strlen(package_buffer[i]) + strlen(dram_buffer[i]) + strlen(cpu_buffer[i]) + 4;	

				/*Insert socket number*/	
				
				break;
			case SANDYBRIDGE:
			case IVYBRIDGE:


				result = read_msr(fd[i],MSR_PP1_ENERGY_STATUS);
				pp1[i] = (double) result *rapl_unit.energy;

				sprintf(gpu_buffer[i], "%f", pp1[i]);

				info_size += strlen(package_buffer[i]) + strlen(gpu_buffer[i]) + strlen(cpu_buffer[i]) + 4;	
				
		}

		ener_info = (char *) malloc(info_size);
	}
}


JNIEXPORT jstring JNICALL Java_EnergyCheckUtils_EnergyStatCheck(JNIEnv *env,
		jclass jcls) {
	jstring ener_string;
	char gpu_buffer[num_pkg][60]; 
	char dram_buffer[num_pkg][60]; 
	char cpu_buffer[num_pkg][60]; 
	char package_buffer[num_pkg][60];
	int dram_num = 0L;
	int cpu_num = 0L;
	int package_num = 0L;
	int gpu_num = 0L;
	//construct a string
	//char *ener_info;
	int info_size;
	int i;
	int offset = 0;

	initialize_energy_info(gpu_buffer, dram_buffer, cpu_buffer, package_buffer);

	for(i = 0; i < num_pkg; i++) {
		//allocate space for string
		//printf("%" PRIu32 "\n", cpu_model);
		switch(cpu_model) {
			case SANDYBRIDGE_EP:
			case HASWELL1:
			case HASWELL2:
			case HASWELL3:
			case HASWELL_EP:
			case SKYLAKE1:
			case SKYLAKE2:
			case BROADWELL:
			case BROADWELL2:

				//copy_to_string(ener_info, dram_buffer, dram_num, cpu_buffer, cpu_num, package_buffer, package_num, i, &offset);
				/*Insert socket number*/	
				dram_num = strlen(dram_buffer[i]);
				cpu_num = strlen(cpu_buffer[i]);
				package_num = strlen(package_buffer[i]);
				
				memcpy(ener_info + offset, &dram_buffer[i], dram_num);
				//split sigh
				ener_info[offset + dram_num] = '#';
				memcpy(ener_info + offset + dram_num + 1, &cpu_buffer[i], cpu_num);
				ener_info[offset + dram_num + cpu_num + 1] = '#';
				if(i < num_pkg - 1) {
					memcpy(ener_info + offset + dram_num + cpu_num + 2, &package_buffer[i], package_num);
					offset += dram_num + cpu_num + package_num + 2;
					if(num_pkg > 1) {
						ener_info[offset] = '@';
						offset++;
					}
				} else {
					memcpy(ener_info + offset + dram_num + cpu_num + 2, &package_buffer[i], package_num + 1);
				}
				
				break;	
			case SANDYBRIDGE:
			case IVYBRIDGE:

				gpu_num = strlen(gpu_buffer[i]);		
				cpu_num = strlen(cpu_buffer[i]);
				package_num = strlen(package_buffer[i]);

				//copy_to_string(ener_info, gpu_buffer, gpu_num, cpu_buffer, cpu_num, package_buffer, package_num, i, &offset);
				memcpy(ener_info + offset, &gpu_buffer[i], gpu_num);
				//split sign
				ener_info[offset + gpu_num] = '#';
				memcpy(ener_info + offset + gpu_num + 1, &cpu_buffer[i], cpu_num);
				ener_info[offset + gpu_num + cpu_num + 1] = '#';
				if(i < num_pkg - 1) {
					memcpy(ener_info + offset + gpu_num + cpu_num + 2, &package_buffer[i], package_num);
					offset += gpu_num + cpu_num + package_num + 2;
					if(num_pkg > 1) {
						ener_info[offset] = '@';
						offset++;
					}
				} else {
					memcpy(ener_info + offset + gpu_num + cpu_num + 2, &package_buffer[i],
							package_num + 1);
				}
				
				break;
		default:
				printf("non of archtectures are detected\n");
				break;


		}
	}

	ener_string = (*env)->NewStringUTF(env, ener_info);	
	free(ener_info);
	return ener_string;

}
JNIEXPORT void JNICALL Java_EnergyCheckUtils_ProfileDealloc
   (JNIEnv * env, jclass jcls) {
	int i;
	free(fd);	
	free(parameters);
}

