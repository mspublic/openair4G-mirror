/*******************************************************************************
 * 
 E *urecom OpenAirInterface
 Copyright(c) 1999 - 2011 Eurecom
 
 This program is free software; you can redistribute it and/or modify it
 under the terms and conditions of the GNU General Public License,
 version 2, as published by the Free Software Foundation.
 
 This program is distributed in the hope it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 more details.
 
 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 
 The full GNU General Public License is included in this distribution in
 the file called "COPYING".
 
 Contact Information
 Openair Admin: openair_admin@eurecom.fr
 Openair Tech : openair_tech@eurecom.fr
 Forums       : http://forums.eurecom.fsr/openairinterface
 Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France
 
 *******************************************************************************/

/*! \file vcd_signal_dumper.c
 * \brief Dump functions calls and variables to VCD file. Use GTKWave to display this file.
 * \author S. Roux
 * \date 2012
 * \version 0.1
 * \company Eurecom
 * \email: roux@eurecom.fr
 * \note
 * \warning
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <error.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>

#include "vcd_signal_dumper.h"

#define VCDSIGNALDUMPER_VERSION_MAJOR 0
#define VCDSIGNALDUMPER_VERSION_MINOR 1

// Global variable. If the VCD option is set at execution time, output VCD trace. Otherwise this module has no effect.
int ouput_vcd = 0;

struct vcd_module_s {
    const char     *name;
    int             number_of_signals;
    const char    **signals_names;
    vcd_signal_type signal_type;
    int             signal_size;
} vcd_module_s;

const char* eurecomVariablesNames[] = {
    "frame_number",
    "slot_number",
};

const char* eurecomFunctionsNames[] = {
    "macxface_macphy_init",
    "macxface_macphy_exit",
    "macxface_eNB_dlsch_ulsch_scheduler",
    "macxface_fill_rar",
    "macxface_terminate_ra_proc",
    "macxface_initiate_ra_proc",
    "macxface_cancel_ra_proc",
    "macxface_get_dci_sdu",
    "macxface_get_dlsch_sdu",
    "macxface_rx_sdu",
    "macxface_mrbch_phy_sync_failure",
    "macxface_SR_indication",
    "macxface_phy_config_sib1_eNB",
    "macxface_phy_config_sib2_eNB",
    "macxface_phy_config_dedicated_eNB",
    "macxface_out_of_sync_ind",
    "macxface_ue_decode_si",
    "macxface_ue_send_sdu",
    "macxface_ue_get_sdu",
    "macxface_ue_get_rach",
    "macxface_ue_process_rar",
    "macxface_ue_scheduler",
    "macxface_ue_get_sr",
    "lte_ue_measurement_procedures",
    "lte_ue_pdcch_procedures",
    "lte_ue_pbcch_procedures",
    "phy_procedures_ue_tx",
    "phy_procedures_ue_rx",
    "phy_procedures_eNB_lte",
    "phy_procedures_UE_lte",
    "emu_transport",
    "log_record",
};

struct vcd_module_s vcd_modules[VCD_SIGNAL_DUMPER_MODULE_END] = {
    { "variables", VCD_SIGNAL_DUMPER_VARIABLES_END, eurecomVariablesNames, VCD_REAL, 64 },
    { "functions", VCD_SIGNAL_DUMPER_FUNCTIONS_END, eurecomFunctionsNames, VCD_WIRE, 1 },
//    { "ue_procedures_functions", VCD_SIGNAL_DUMPER_UE_PROCEDURES_FUNCTIONS_END, eurecomUEFunctionsNames, VCD_WIRE, 1 },
};

FILE *vcd_fd = NULL;

#if defined(ENABLE_USE_CPU_EXECUTION_TIME)
struct timespec     g_time_start;
#endif

void vcd_signal_dumper_init(void)
{
    char filename[] = "openair_vcd_dump.vcd";
    if (ouput_vcd) {
        if ((vcd_fd = fopen(filename, "w+")) == NULL)
        {
            perror("vcd_signal_dumper_init: cannot open file");
            return;
        }
#if defined(ENABLE_USE_CPU_EXECUTION_TIME)
        clock_gettime(CLOCK_MONOTONIC, &g_time_start);
#endif
        vcd_signal_dumper_create_header();
    }
}

void vcd_signal_dumper_close(void)
{
    if (ouput_vcd) {
        if (vcd_fd != NULL)
        {
            fclose(vcd_fd);
            vcd_fd = NULL;
        }
    }
}

static inline void vcd_signal_dumper_print_time_since_start(void)
{
#if defined(ENABLE_USE_CPU_EXECUTION_TIME)
    if (vcd_fd != NULL)
    {
        struct timespec time;
        long long unsigned int nanosecondsSinceStart;
        long long unsigned int secondsSinceStart;

        clock_gettime(CLOCK_MONOTONIC, &time);

        /* Get current execution time in nanoseconds */
        nanosecondsSinceStart = (long long unsigned int)((time.tv_nsec - g_time_start.tv_nsec));
        secondsSinceStart     = (long long unsigned int)time.tv_sec - (long long unsigned int)g_time_start.tv_sec;
        /* Write time in nanoseconds */
        fprintf(vcd_fd, "#%llu\n", nanosecondsSinceStart + (secondsSinceStart * 1000000000UL));
    }
#endif
}

void vcd_signal_dumper_create_header(void)
{
    if (ouput_vcd) {
        struct tm *pDate;
        time_t intps;

        intps = time(NULL);
        pDate = localtime(&intps);

        if (vcd_fd != NULL)
        {
            int i, j;
            fprintf(vcd_fd, "$date\n\t%s$end\n", asctime(pDate));
            // Display version
            fprintf(vcd_fd, "$version\n\tVCD plugin ver%d.%d\n$end\n", VCDSIGNALDUMPER_VERSION_MAJOR, VCDSIGNALDUMPER_VERSION_MINOR);
            // Init timescale, here = 1ns
            fprintf(vcd_fd, "$timescale 1 ns $end\n");

            /* Initialize each module definition */
            for(i = 0; i < VCD_SIGNAL_DUMPER_MODULE_END; i++) {
                struct vcd_module_s *module;
                module = &vcd_modules[i];
                fprintf(vcd_fd, "$scope module %s $end\n", module->name);
                /* Declare each signal as defined in array */
                for (j = 0; j < module->number_of_signals; j++) {
                    const char *signal_name;
                    signal_name = module->signals_names[j];
                    if (VCD_WIRE == module->signal_type) {
                        fprintf(vcd_fd, "$var wire %d %s_w %s $end\n", module->signal_size, signal_name, signal_name);
                    } else  if (VCD_REAL == module->signal_type) {
                        fprintf(vcd_fd, "$var real %d %s_r %s $end\n", module->signal_size, signal_name, signal_name);
                    } else {
                        // Handle error here
                    }
                }
                fprintf(vcd_fd, "$upscope $end\n");
            }

            /* Init variables and functions to 0 */
            fprintf(vcd_fd, "$dumpvars\n");
            for(i = 0; i < VCD_SIGNAL_DUMPER_MODULE_END; i++) {
                struct vcd_module_s *module;
                module = &vcd_modules[i];
                /* Declare each signal as defined in array */
                for (j = 0; j < module->number_of_signals; j++) {
                    const char *signal_name;
                    signal_name = module->signals_names[j];
                    if (VCD_WIRE == module->signal_type) {
                        fprintf(vcd_fd, "0%s_w $end\n", signal_name);
                    } else  if (VCD_REAL == module->signal_type) {
                        fprintf(vcd_fd, "r0 %s_r $end\n", signal_name);
                    } else {
                        // Handle error here
                    }
                }
            }
            fprintf(vcd_fd, "$end\n");
            fprintf(vcd_fd, "$enddefinitions $end\n\n");
            fflush(vcd_fd);
        }
    }
}

void vcd_signal_dumper_dump_variable_by_name(vcd_signal_dump_variables variable_name,
                                             unsigned long             value)
{
    if (ouput_vcd) {
        assert(variable_name < VCD_SIGNAL_DUMPER_VARIABLES_END);
        assert(variable_name >= 0);

        if (vcd_fd != NULL)
        {
            vcd_signal_dumper_print_time_since_start();

            /* Set variable to value */
            fprintf(vcd_fd, "r%lu %s_r\n", value, eurecomVariablesNames[variable_name]);
            fflush(vcd_fd);
        }
    }
}

void vcd_signal_dumper_dump_function_by_name(vcd_signal_dump_functions  function_name,
                                             vcd_signal_dump_in_out     in_out)
{
    if (ouput_vcd) {
        assert(function_name < VCD_SIGNAL_DUMPER_FUNCTIONS_END);
        assert(function_name >= 0);

        if (vcd_fd != NULL)
        {
            vcd_signal_dumper_print_time_since_start();

            /* Check if we are entering or leaving the function ( 0 = leaving, 1 = entering) */
            if (in_out == VCD_FUNCTION_IN)
                /* Set event to 1 */
                fprintf(vcd_fd, "1%s_w\n", eurecomFunctionsNames[function_name]);
            else
                fprintf(vcd_fd, "0%s_w\n", eurecomFunctionsNames[function_name]);
            fflush(vcd_fd);
        }
    }
}
