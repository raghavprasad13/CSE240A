//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Raghav Prasad";
const char *studentID   = "A5902163";
const char *email       = "rprasad@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

// GShare
uint32_t g_history;
uint8_t *gshare_bht;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor

void gshare_init() {
  int gshare_bht_size = 1 << ghistoryBits;
  gshare_bht = (uint8_t *)malloc(gshare_bht_size * sizeof(uint8_t));
  for(int i = 0; i < gshare_bht_size; i++) {
    gshare_bht[i] = SN;
  }
  g_history = 0;
}

void tournament_init() {

}

void perceptron_init() {

}

void init_predictor()
{
  switch(bpType) {
    case GSHARE:
      gshare_init();
      break;
    case TOURNAMENT:
      tournament_init();
      break;
    case CUSTOM:
      perceptron_init();
      break;
    default:
      break;
  }
}

uint8_t gshare_prediction(uint32_t pc) {
  uint32_t mask = UINT32_MAX >> (32 - ghistoryBits);
  uint32_t pc_lower = pc & mask;
  uint32_t g_history_lower = g_history & mask;
  uint32_t idx = pc_lower ^ g_history_lower;

  switch(gshare_bht[idx]) {
    case WN:
    case SN:
      return NOTTAKEN;
    case WT:
    case ST:
      return TAKEN;
    default:
      break;
  }

  return NOTTAKEN;
}

uint8_t tournament_prediction(uint32_t pc) {
  return NOTTAKEN;
}

uint8_t perceptron_prediction(uint32_t pc) {
  return NOTTAKEN;
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t make_prediction(uint32_t pc) {
  // Make a prediction based on the bpType
  switch(bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare_prediction(pc);
    case TOURNAMENT:
      return tournament_prediction(pc);
    case CUSTOM:
      return perceptron_prediction(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

void gshare_train(uint32_t pc, uint8_t outcome) {
  uint32_t mask = UINT32_MAX >> (32 - ghistoryBits);
  uint32_t pc_lower = pc & mask;
  uint32_t g_history_lower = g_history & mask;
  uint32_t idx = pc_lower ^ g_history_lower;

  switch(gshare_bht[idx]) {
    case WN:
      if(outcome == NOTTAKEN) {
        gshare_bht[idx] = SN;
      } else {
        gshare_bht[idx] = WT;
      }
      break;
    case SN:
      if(outcome == TAKEN) {
        gshare_bht[idx] = WN;
      }
      break;
    case WT:
      if(outcome == NOTTAKEN) {
        gshare_bht[idx] = WN;
      } else {
        gshare_bht[idx] = ST;
      }
      break;
    case ST:
      if(outcome == NOTTAKEN) {
        gshare_bht[idx] = WT;
      }
      break;
    default:
      break;
    }

    g_history = (g_history << 1) | outcome;
}

void tournament_train(uint32_t pc, uint8_t outcome) {
}

void perceptron_train(uint32_t pc, uint8_t outcome) {
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
void train_predictor(uint32_t pc, uint8_t outcome) {
  //
  //TODO: Implement Predictor training
  //
  switch(bpType) {
  case GSHARE:
    gshare_train(pc, outcome);
    break;
  case TOURNAMENT:
    tournament_train(pc, outcome);
    break;
  case CUSTOM:
    perceptron_train(pc, outcome);
    break;
  default:
    break;
  }
}

void cleanup() {
  switch(bpType) {
    case GSHARE:
      free(gshare_bht);
      break;
    case TOURNAMENT:
      // TODO
      break;
    case CUSTOM:
      // TODO
      break;
    default:
      break;
  }
}
