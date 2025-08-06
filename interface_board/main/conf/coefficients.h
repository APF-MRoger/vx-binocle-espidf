#pragma once
#define COEFF_COOLANT_DEGC_TO_DUTY_M (double)(100.0 / 71.0)
#define COEFF_COOLANT_DEGC_TO_DUTY_P (double)(-100.0 * 64.0 / 71.0)
#define COEFF_DUTY_TO_COOLANT_DEGC_M (double)(71.0 / 100.0)
#define COEFF_DUTY_TO_COOLANT_DEGC_P (double)(64.0)

#define COEFF_RPM_TO_FREQ_M (double)(1.0 / 30.0)
#define COEFF_RPM_TO_FREQ_P (double)(0)
#define COEFF_FREQ_TO_RPM_M (double)(30.0)
#define COEFF_FREQ_TO_RPM_P (double)(0)

#define COEFF_SPEED_KPH_TO_FREQ_M (double)(31285.0 / 7651.0)
#define COEFF_SPEED_KPH_TO_FREQ_P (double)(0.0)
#define COEFF_FREQ_TO_SPEED_KPH_M (double)(7651.0 / 31285.0)
#define COEFF_FREQ_TO_SPEED_KPH_P (double)(0.0)

#define COEFF_SPEED_MPH_TO_FREQ_M (double)(17182.0 / 2611.0)
#define COEFF_SPEED_MPH_TO_FREQ_P (double)(0.0)
#define COEFF_FREQ_TO_SPEED_MPH_M (double)(2611.0/17182.0)
#define COEFF_FREQ_TO_SPEED_MPH_P (double)(0.0)