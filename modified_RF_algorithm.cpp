/*
 * Created by Robert Fraczkiewicz, 12/2017
 * Modified by somna to use realtime/circular buffer (2024/12)
 * New signal processing methodology for obtaining heart rate and SpO2 data 
 * from the MAX30102 sensor manufactured by MAXIM Integrated Products, Inc.
 */
/*******************************************************************************
* Copyright (C) 2017 Robert Fraczkiewicz, All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL ROBERT FRACZKIEWICZ BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Robert Fraczkiewicz retains all
* ownership rights.
*******************************************************************************
*/
#include "algorithm_by_RF.h"
#include <math.h>

void rt_rf_heart_rate(uint32_t *pun_ir_buffer, int32_t n_ir_buffer_length, int32_t p_buffer_zero_pos, uint32_t *pun_red_buffer,
                int32_t *pn_heart_rate, int8_t *pch_hr_valid, float *ratio, float *correl)
/**
* \brief        Calculate the heart rate and SpO2 level, Robert Fraczkiewicz version
* \par          Details
*               By detecting  peaks of PPG cycle and corresponding AC/DC of red/infra-red signal, the xy_ratio for the SPO2 is computed.
*
* \param[in]    *pun_ir_buffer           - IR sensor data buffer
* \param[in]    n_ir_buffer_length       - IR sensor data buffer length
* \param[in]    p_buffer_zero_pos        - 0 index in the circular buffer
* \param[in]    *pun_red_buffer          - Red sensor data buffer
* \param[out]    *pn_heart_rate          - Calculated heart rate value
* \param[out]    *pch_hr_valid           - 1 if the calculated heart rate value is valid
*
* \retval       None
*/
{
  int32_t k; int32_t i;
  static int32_t n_last_peak_interval=LOWEST_PERIOD;
  float f_ir_mean,f_red_mean,f_ir_sumsq,f_red_sumsq;
  float f_y_ac, f_x_ac, xy_ratio;
  float beta_ir, beta_red, x;
  float an_x[BUFFER_SIZE], *ptr_x; //ir
  float an_y[BUFFER_SIZE], *ptr_y; //red

  // calculates DC mean and subtracts DC from ir and red
  f_ir_mean=0.0; 
  f_red_mean=0.0;
  for (i=0; i<n_ir_buffer_length; ++i) {
    k = p_buffer_zero_pos+i; if (k>=n_ir_buffer_length) k-= n_ir_buffer_length;
    f_ir_mean += pun_ir_buffer[k];
    f_red_mean += pun_red_buffer[k];
  }
  f_ir_mean=f_ir_mean/n_ir_buffer_length ;
  f_red_mean=f_red_mean/n_ir_buffer_length ;
  
  // remove DC 
  for (i=0,ptr_x=an_x,ptr_y=an_y; i<n_ir_buffer_length; ++i,++ptr_x,++ptr_y) {
    k = p_buffer_zero_pos+i; if (k>=n_ir_buffer_length) k-= n_ir_buffer_length;
    *ptr_x = pun_ir_buffer[k] - f_ir_mean;
    *ptr_y = pun_red_buffer[k] - f_red_mean;
  }
  
  // RF, remove linear trend (baseline leveling)
  beta_ir = rf_linear_regression_beta(an_x, mean_X, sum_X2);
  beta_red = rf_linear_regression_beta(an_y, mean_X, sum_X2);
  for(k=0,x=-mean_X,ptr_x=an_x,ptr_y=an_y; k<n_ir_buffer_length; ++k,++x,++ptr_x,++ptr_y) {
    *ptr_x -= beta_ir*x;
    *ptr_y -= beta_red*x;
  }
  
    // For SpO2 calculate RMS of both AC signals. In addition, pulse detector needs raw sum of squares for IR
  f_y_ac=rf_rms(an_y,n_ir_buffer_length,&f_red_sumsq);
  f_x_ac=rf_rms(an_x,n_ir_buffer_length,&f_ir_sumsq);

  // Calculate Pearson correlation between red and IR
  *correl=rf_Pcorrelation(an_x, an_y, n_ir_buffer_length)/sqrt(f_red_sumsq*f_ir_sumsq);

  // Find signal periodicity
  if(*correl>=min_pearson_correlation) {
    // At the beginning of oximetry run the exact range of heart rate is unknown. This may lead to wrong rate if the next call does not find the _first_
    // peak of the autocorrelation function. E.g., second peak would yield only 50% of the true rate. 
    if(LOWEST_PERIOD==n_last_peak_interval) 
      rf_initialize_periodicity_search(an_x, BUFFER_SIZE, &n_last_peak_interval, HIGHEST_PERIOD, min_autocorrelation_ratio, f_ir_sumsq);
    // RF, If correlation os good, then find average periodicity of the IR signal. If aperiodic, return periodicity of 0
    if(n_last_peak_interval!=0)
      rf_signal_periodicity(an_x, BUFFER_SIZE, &n_last_peak_interval, LOWEST_PERIOD, HIGHEST_PERIOD, min_autocorrelation_ratio, f_ir_sumsq, ratio);
  } else n_last_peak_interval=0;

  // Calculate heart rate if periodicity detector was successful. Otherwise, reset peak interval to its initial value and report error.
  if(n_last_peak_interval!=0) {
    *pn_heart_rate = (int32_t)(FS60/n_last_peak_interval);
    *pch_hr_valid  = 1;
  } else {
    n_last_peak_interval=LOWEST_PERIOD;
    *pn_heart_rate = -999; // unable to calculate because signal looks aperiodic
    *pch_hr_valid  = 0;
    return;
  }
}