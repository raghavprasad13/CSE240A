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
uint32_t glob_hist;
uint8_t *gshare_bht;

// Tournament
uint8_t *local_hist_table;
uint8_t *local_pred_table;
uint8_t *global_pred_table;
uint8_t *choice_pred_table;
uint32_t pred_hist;

// Perceptron
int **perceptrons;
int weights;
int thresh;
int perceptron_hist_len;
uint64_t perceptron_glob_hist;
#define THRESH_CONST_1 1.93
#define THRESH_CONST_2 14

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
  glob_hist = 0;
}

void tournament_init() {
  int num_global_entries = 1 << ghistoryBits;
  int num_local_pred_entries = 1 << lhistoryBits;
  int num_local_hist_entries = 1 << pcIndexBits;

  global_pred_table = (uint8_t *)malloc(num_global_entries * sizeof(uint8_t));
  local_pred_table = (uint8_t *)malloc(num_local_pred_entries * sizeof(uint8_t));
  local_hist_table = (uint8_t *)malloc(num_local_hist_entries * sizeof(uint8_t));
  choice_pred_table = (uint8_t *)malloc(num_global_entries * sizeof(uint8_t));

  for(int i = 0; i < num_global_entries; i++) {
    global_pred_table[i] = WN;
    choice_pred_table[i] = WT;
  }

  for(int i = 0; i < num_local_pred_entries; i++) {
    local_pred_table[i] = WN;
  }

  for(int i = 0; i < num_local_hist_entries; i++) {
    local_hist_table[i] = SN;
  }

  pred_hist = 0;
}

void perceptron_init() {
  perceptrons = (int **)malloc(weights * sizeof(int *));
  for(int i = 0; i < weights; i++) {
    perceptrons[i] = (int *)malloc((perceptron_hist_len + 1) * sizeof(int));
    for(int j = 0; j < perceptron_hist_len + 1; j++) {
      perceptrons[i][j] = TAKEN;
    }
  }

  thresh = (THRESH_CONST_1 * perceptron_hist_len) + THRESH_CONST_2;
  perceptron_glob_hist = 0;
}

void init_predictor() {
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
  uint32_t glob_hist_lower = glob_hist & mask;
  uint32_t idx = pc_lower ^ glob_hist_lower;

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

uint32_t get_global_and_local_pred(uint32_t pc) {
  uint32_t pc_mask = UINT32_MAX >> (32 - pcIndexBits);
  uint32_t pc_lower = pc & pc_mask;
  uint32_t local_pred_table_idx = local_hist_table[pc_lower];
  uint8_t local_pred_table_entry = local_pred_table[local_pred_table_idx];

  uint32_t g_mask = UINT32_MAX >> (32 - ghistoryBits);
  uint32_t glob_hist_lower = pred_hist & g_mask;
  uint8_t global_pred_table_entry = global_pred_table[glob_hist_lower];

  uint32_t global_pred = TAKEN;
  uint32_t local_pred = TAKEN;
  if(global_pred_table_entry == WN || global_pred_table_entry == SN)
    global_pred = NOTTAKEN;

  if(local_pred_table_entry == WN || local_pred_table_entry == SN)
    local_pred = NOTTAKEN;

  return (global_pred << 1) | local_pred;
}

uint8_t tournament_prediction(uint32_t pc) {
  uint32_t global_local_pred = get_global_and_local_pred(pc);
  if(global_local_pred == ST)
    return TAKEN;
  if(global_local_pred == SN)
    return NOTTAKEN;
  if(global_local_pred == WN) {
    if(choice_pred_table[pred_hist] > 1)
      return NOTTAKEN;
    return TAKEN;
  }
  if(choice_pred_table[pred_hist] > 1)
    return TAKEN;
  return NOTTAKEN;
}

uint8_t perceptron_prediction(uint32_t pc) {
  int idx = (pc >> 2) % weights;
  int ans = perceptrons[idx][0];
  int sign = 1;
  for(int i = 0; i < perceptron_hist_len + 1; i++) {
    if((perceptron_glob_hist & (1 << i)) != 0)
      sign = 1;
    else
      sign = -1;
    ans += (sign * perceptrons[idx][i]);
  }

  if(ans > 0)
    return TAKEN;
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

  // If there is not a compatible bpType then return NOTTAKEN
  return NOTTAKEN;
}

void gshare_train(uint32_t pc, uint8_t outcome) {
  uint32_t mask = UINT32_MAX >> (32 - ghistoryBits);
  uint32_t pc_lower = pc & mask;
  uint32_t glob_hist_lower = glob_hist & mask;
  uint32_t idx = pc_lower ^ glob_hist_lower;

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

    glob_hist = (glob_hist << 1) | outcome;
}

void tournament_train(uint32_t pc, uint8_t outcome) {
  uint32_t pc_mask = UINT32_MAX >> (32 - pcIndexBits);
  uint32_t pc_lower = pc & pc_mask;
  uint32_t local_pred_table_idx = local_hist_table[pc_lower];

  int num_global_entries = 1 << ghistoryBits;
  int num_local_hist_entries = 1 << pcIndexBits;

  if(outcome == TAKEN) {
    if(local_pred_table[local_pred_table_idx] < 3)
      local_pred_table[local_pred_table_idx]++;
    if(global_pred_table[pred_hist] < 3)
      global_pred_table[pred_hist]++;
  } else {
    if(local_pred_table[local_pred_table_idx] > 0)
      local_pred_table[local_pred_table_idx]--;
    if(global_pred_table[pred_hist] > 0)
      global_pred_table[pred_hist]--;
  }

  local_hist_table[pc_lower] = (local_hist_table[pc_lower] << 1) | outcome;
  local_hist_table[pc_lower] &= (num_local_hist_entries - 1); // Not sure about this

  uint32_t choice = get_global_and_local_pred(pc);
  int pred_choice = choice_pred_table[pred_hist];
  if(choice == WN) {
    if(outcome == TAKEN) {
      if(pred_choice > 0)
        choice_pred_table[pred_hist] = pred_choice - 1;
    } else {
      if(pred_choice < 3)
        choice_pred_table[pred_hist] = pred_choice + 1;
    }
  } else if(choice == WT) {
    if(outcome == TAKEN) {
      if(pred_choice < 3)
        choice_pred_table[pred_hist] = pred_choice + 1;
    } else {
      if(pred_choice > 0)
        choice_pred_table[pred_hist] = pred_choice - 1;
    }
  }

  pred_hist = (pred_hist << 1) | outcome;
  pred_hist &= (num_global_entries - 1);  // Not sure about this
}

void perceptron_train(uint32_t pc, uint8_t outcome) {
  int idx = (pc >> 2) % weights;
  int ans = perceptrons[idx][0];
  int sign = 1;
  for(int i = 0; i < perceptron_hist_len + 1; i++) {
    if((perceptron_glob_hist & (1 << i)) != 0)
      sign = 1;
    else
      sign = -1;
    ans += (sign * perceptrons[idx][i]);
  }

  int train_sign = 1;
  if(outcome == NOTTAKEN)
    train_sign = -1;

  if((ans > 0 && outcome == NOTTAKEN) || (abs(ans) < thresh)) {
    perceptrons[idx][0] += train_sign;
    for(int i = 1; i <= perceptron_hist_len; i++) {
      if((perceptron_glob_hist & (1 << i)) != 0)
        perceptrons[idx][i] += train_sign;
      else
        perceptrons[idx][i] -= train_sign;
    }
  }

  perceptron_glob_hist = (perceptron_glob_hist << 1) | outcome;
  perceptron_glob_hist &= (1 << perceptron_hist_len) - 1;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
void train_predictor(uint32_t pc, uint8_t outcome) {
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
      free(local_hist_table);
      free(local_pred_table);
      free(global_pred_table);
      free(choice_pred_table);
      break;
    case CUSTOM:
      for(int i = 0; i < weights; i++) {
        free(perceptrons[i]);
      }
      free(perceptrons);
      break;
    default:
      break;
  }
}
