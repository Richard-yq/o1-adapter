/*
* Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
* contributor license agreements.  See the NOTICE file distributed with
* this work for additional information regarding copyright ownership.
* The OpenAirInterface Software Alliance licenses this file to You under
* the OAI Public License, Version 1.1  (the "License"); you may not use this file
* except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.openairinterface.org/?page_id=698
*
* Copyright: Fraunhofer Heinrich Hertz Institute
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*-------------------------------------------------------------------------------
* For more information about the OpenAirInterface (OAI) Software Alliance:
*      contact@openairinterface.org
*/

#pragma once

#include <stdint.h>

typedef struct oai_additional_data {
    char *frameType;
    int bandNumber;
    int numUes;
    int *ues;
    int load;
} oai_additional_data_t;

typedef struct oai_config_data {
    int gnbId;
    char *gnbName;
    char *vendor;
} oai_device_data_t;

typedef struct oai_bwp_data {
    int isInitialBwp;
    int numberOfRBs;
    int startRB;
    int subCarrierSpacing;
} oai_bwp_data_t;

typedef struct oai_nrcelldu_data {
    int ssbFrequency;
    int arfcnDL;
    int bSChannelBwDL;
    int arfcnUL;
    int bSChannelBwUL;
    int nRPCI;
    int nRTAC;

    char* mcc;
    char* mnc;
    uint32_t sd;
    uint32_t sst;
} oai_nrcelldu_data_t;

typedef struct oai_data {
    oai_bwp_data_t bwp[2];
    oai_nrcelldu_data_t nrcelldu;
    oai_device_data_t device_data;
    oai_additional_data_t additional_data;
} oai_data_t;

oai_data_t *oai_data_parse_json(const char *json);
void oai_data_print(const oai_data_t *data);
void oai_data_free(oai_data_t *data);
