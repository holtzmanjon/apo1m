/****************************************************************************/
/*                                                                       */
/*      Module:         shutdown.cpp                                     */
/*                                                                       */
/*      Purpose:        module to monitor weather, watchdog, etc and shutdown */
/*                                                                       */
/****************************************************************************/
/*                                                                       */
/*                    PROPERTY OF AUTOSCOPE CORPORATION                  */
/*                        2637 Midpoint Dr., Suite D                     */
/*                         Fort Collins, CO  80525                       */
/*                                                                       */
/*                            Copyright 1995                             */
/*              Unauthorized duplication or use is prohibited.           */
/*                                                                       */
/*      Author:         M. Donahue                                       */
/*                                                                       */
/****************************************************************************/
#include <stdlib.h>
#include <iostream.h>
#include <dos.h>
#include <string.h>

#include "shutdown.h"
#include "evtimer.h"
#include "globals.h"
#include "tcs.h"
#include "ocs.h"
#include "status.h"
#include "systimer.h"
#include "sounds.h"
#include "weather.h"
#include "io.h"
#include "pcx.h"

void remoteremove(char *);
char *louverfile = TOCC"\\tocc\\louver_open.dat";
char *louverfile1 = TOCC"\\tocc\\louver_override.dat";

//------------------------------------------------------------------
//      Name.......:    check_priority
//
//  Purpose....:   monitor the watchdog and rain detector to see if we
//                 need to shutdown.  if so, stop all axes, shutter, and dome,
//                 and set the shutdown flag.
//
//                 Also, check for user abort request
//
//      Input......:    none
//
//      Output.....:    FALSE if we need to shut down
//                 error code
//
//------------------------------------------------------------------
BOOL check_priority(unsigned& status)
	{
		status = TCSERR_OK;

		#ifdef no_hardware
		return TRUE;
		#else
		// if there is no OCS, exit
		if (!ocs_installed())
			return TRUE;

// check the watchdog - ignore the status code if shutdowns are disabled
		if (SysTimer[SYSTMR_WDOG_CHECK].Expired())
			ocs_reset_watchdog();

		// don't shutdown if we are already in the shutdown mode
		if (G->shutdown_state)
			return TRUE;

		// don't do the other checks if shutdowns are disabled
		if (!sysGlobal->shutdown_enabled)
			return TRUE;

		// check the watchdog timer
		if (!watchdog_ok(status)) {
				G->need_to_shutdown = TRUE;
				G->shutdown_state |= SDC_WDOG;
				return FALSE;
		}

		// check the rain detector
		if (ocs_rain_detector_active()) {
				status = TCSERR_WEATHERSHUT;
				G->need_to_shutdown = TRUE;
				G->shutdown_state |= SDC_RAIN;
				return FALSE;
		}

		return TRUE;
		#endif
	}

static int old_shutdown_state = 0;

void do_shutdown()
	{
	    if (G->shutdown_state != old_shutdown_state) {
		  // log the shutdown state
	  	  sprintf(outbuf,"Shutdown state: 0x%x",G->shutdown_state);
		  writelog(outbuf,0);
	  	  sprintf(outbuf,"Shutdown state: 0x%x 0x%x",G->shutdown_state,old_shutdown_state);
		  writeline(outbuf,1);
		  old_shutdown_state = G->shutdown_state;
  
		  // print informational message
		  //error_sound();
		  //Delay(25);
		  //error_sound();
		  sprintf(outbuf, "The system will shutdown due to:\n");
		  if (G->shutdown_state & SDC_RAIN)
		    sprintf(outbuf+strlen(outbuf),"- rain detection\n");
		  if (G->shutdown_state & SDC_WIND)
		    sprintf(outbuf+strlen(outbuf), "- wind speed\n");
		  if (G->shutdown_state & SDC_HUMIDITY)
		    sprintf(outbuf+strlen(outbuf), "- humiditiy\n");
		  if (G->shutdown_state & SDC_OUTTEMP)
		    sprintf(outbuf+strlen(outbuf), "- outside temperature\n");
		  if (G->shutdown_state & SDC_CABTEMP)
		    sprintf(outbuf+strlen(outbuf), "- cabinet temperature\n");
		  if (G->shutdown_state & SDC_WDOG)
		    sprintf(outbuf+strlen(outbuf), "- watchdog timer failure\n");
		  if (G->shutdown_state & SDC_UPS_BATT)
		    sprintf(outbuf+strlen(outbuf), "- UPS low battery\n");
		  if (G->shutdown_state & SDC_UPS_LINE)
	  	    sprintf(outbuf+strlen(outbuf), "- UPS commercial power failure\n");
	  	  if (G->shutdown_state & SDC_WSFAIL)
		    sprintf(outbuf+strlen(outbuf), "- max weather station failures reached\n");
		  if (G->shutdown_state & SDC_35M_CLOSED)
		    sprintf(outbuf+strlen(outbuf), "- 3.5m dome closed\n");
		  if (G->shutdown_state & SDC_35M_NOT_OPEN)
		    sprintf(outbuf+strlen(outbuf), "- 3.5m dome not reading open\n");
		  if (G->shutdown_state & SDC_NO_NETWORK)
		    sprintf(outbuf+strlen(outbuf), "- network appears to be down\n");

	  	  writeline(outbuf,0);
                }

		// clear need_to_shutdown flag
		G->need_to_shutdown = FALSE;

#ifndef no_hardware
		// Dont need to shutdown if we are already shut down
		// if (G->shutdown_state & SDC_SHUTDOWN) {
		//  writeline("We are already shutdown.",0);
		//  return;
		// }

		// stop all motion
		tcs_telescope_stop(FALSE);  // don't wait for done flags
		ocs_emergency_stop();

		// make sure louvers are slaved to 3.5m
		remove(louverfile);
		// make sure the louvers get closed
		remove(louverfile1);

		// close the dust covers
		if (sysGlobal->mirror_covers_installed &&
		    G->mirror_covers_open) tcs_dust(0);

	    // lower the telescope in altitude
		if (!G->shutdown_state) {
		  char buffer[81];
	  	  sprintf(buffer,"ay ac%ld; vl%ld; ma%ld; gd; id;",
		        sysGlobal->y_acceleration,
		        sysGlobal->y_max_velocity,
		        sysGlobal->y_park_steps);
                  writeline(buffer,1);
	  	  pc38_send_commands(buffer);
		}

		// close the shutter 
		if (G->dome_open && G->lower_dome_open ) 
		    ocs_close_shutters();
                else {
		  if (G->lower_dome_open || G->lower_dome_part_open)
		    ocs_close_lower_shutter();

		  if (G->dome_open || G->dome_part_open )
		    ocs_close_shutter();
                }

		// park the telescope
		if (G->telescope_at_home !=4 && G->telescope_at_home !=3) tcs_telescope_park(0);

		writeline("Shutdown complete.",0);
		// G->shutdown_state |= SDC_SHUTDOWN;

		#endif
	}

void check_sensors()
	{
		#ifndef no_hardware
		UPSPACK ups;

		// if there is no OCS, exit
		if (!ocs_installed())
			return;

                // check ups
		if (sysGlobal->ups_installed) check_ups();

		// watchdog timer
		if (sysGlobal->watchdog_installed) {
			if (G->shutdown_state & SDC_WDOG) {
// we are currently shutdown due, in part, to a timed out watchdog
// timer.  try to clear it and test it again.  if it is clear,
				// clear the watchdog status bit
				ocs_reset_watchdog();
				Delay(10);
				if (!ocs_watchdog_active())
					G->shutdown_state &= ~SDC_WDOG;
			} else if (SysTimer[SYSTMR_WDOG_CHECK].Expired()) {
				ocs_reset_watchdog();
				Delay(10);
				ocs_watchdog_active();
				if (sysGlobal->shutdown_enabled && 
				    ocs_watchdog_active())
				{
				  // the watchdog failed to reset.
				  // set the need to shutdown flag.
				  G->need_to_shutdown = TRUE;
				  G->shutdown_state |= SDC_WDOG;
				}
			}
		}

		// rain detector
		if (sysGlobal->shutdown_enabled && 
		    sysGlobal->rain_detector_installed) {
			if (G->shutdown_state & SDC_RAIN) {
	// we are currently shutdown due, in part, to rain detection.
	// if the detector is no longer active, start the rain clear timer.
			  if (!ocs_rain_detector_active()) {
	// if the rain clear timer has not been started, start it
				if (!SysTimerActive[SYSTMR_RAIN_STOP]) {
				  SysTimer[SYSTMR_RAIN_STOP].
				  NewTimerSecs(sysGlobal->shutdown_delay_rain * 60);
				  SysTimerActive[SYSTMR_RAIN_STOP] = TRUE;
				}

	// if the rain clear timer has expired, the rain has passed
	// so clear the rain bit and stop the timer
				if (SysTimer[SYSTMR_RAIN_STOP].Expired()) {
				  SysTimerActive[SYSTMR_RAIN_STOP] = FALSE;
				  G->shutdown_state &= ~SDC_RAIN;
				}
			  } else {
	// the rain detector is active.  make sure that the clear
	// timer is not running and does not start until the rain
	// detector is inactive
				SysTimerActive[SYSTMR_RAIN_STOP] = FALSE;
			  }
			} else {
			  if (ocs_rain_detector_active()) {
	// the rain detector is active.  set the need to shutdown flag
				G->need_to_shutdown = TRUE;
				G->shutdown_state |= SDC_RAIN;
			  }
			}
		}

		// shutdown if necessary
		if (G->need_to_shutdown)
			do_shutdown();
		#endif
	}

void check_weather()
	{
		#ifndef no_hardware
		unsigned status;
		struct WEATHERPACK weather;

		check_35m();

		// if there is no weather station, exit
		if (!sysGlobal->weather_installed)
			return;

		// if not time to read the weather station, exit
		if (!SysTimer[SYSTMR_WEATHER_CHECK].Expired()) return;

		// read the weather station
		sprintf(outbuf,"Reading the weather station...");
		writeline(outbuf,1);
		status = read_weather_station(weather);
		SysTimer[SYSTMR_WEATHER_CHECK].
			NewTimerSecs(sysGlobal->weather_check_frequency);

		if (status) {
			G->weather_failures++;
			sprintf(outbuf,
			 "Weather station failure. Code: %d\n"
			 " Failures = %d\n"
			 " Max = %d\n",
			 status,G->weather_failures,
			 (int)sysGlobal->max_wstation_failures);
			if (G->weather_failures < 
			 sysGlobal->max_wstation_failures) return;
		} else {
			G->weather_failures = 0;
			// clear station failure condition
			G->shutdown_state &= ~SDC_WSFAIL;       

			// save some of the current readings
			G->current_aux_temp = weather.AuxTemp;
			G->current_cab_temp = weather.CabTemp;
			G->current_out_temp = weather.OutTemp;
			G->current_barometer = weather.Pressure;
			G->current_humidity = (double)weather.Humidity / 100.0;
			G->current_windspeed = weather.WindSpeed;
			G->current_winddir = weather.WindDir;
		}

		// if shutdowns are disabled, exit
		if (!sysGlobal->shutdown_enabled) return;

		// did we reach the maximum number of failures?
		if (G->weather_failures >= sysGlobal->max_wstation_failures) {
			G->need_to_shutdown = TRUE;
			G->shutdown_state |= SDC_WSFAIL;
			if (G->need_to_shutdown) {
				do_shutdown();
				return;
			}
		}

		// check the humidity
		if (G->shutdown_state & SDC_HUMIDITY) {
		 // we are currently shutdown due, in part, to humidity.
		 // if the humidity is ok, start the humidity clear timer.
		 if (weather.Humidity < sysGlobal->shutdown_humidity)
		  {
		  // if the humidity clear timer has not been started, start it
			if (!SysTimerActive[SYSTMR_HUMIDITY_CLEAR]) {
				SysTimer[SYSTMR_HUMIDITY_CLEAR].
				NewTimerSecs(sysGlobal->shutdown_delay_ws * 60);
				SysTimerActive[SYSTMR_HUMIDITY_CLEAR] = TRUE;
			}

		 // if the humidity clear timer has expired, clear the condition
			if (SysTimer[SYSTMR_HUMIDITY_CLEAR].Expired()) {
				SysTimerActive[SYSTMR_HUMIDITY_CLEAR] = FALSE;
				G->shutdown_state &= ~SDC_HUMIDITY;
			}
		  }
		  else {
		   // the humidity is out of range.  make sure that the clear
		   // timer is not running and does not start until the
		   // humidity is within range
			SysTimerActive[SYSTMR_HUMIDITY_CLEAR] = FALSE;
		  }
		}
		else {
		  if (weather.Humidity >= sysGlobal->shutdown_humidity) {
			// the humidity is out of range.  we need to shutdown
				G->need_to_shutdown = TRUE;
				G->shutdown_state |= SDC_HUMIDITY;
		  }
		}

		// check the wind speed
		if (G->shutdown_state & SDC_WIND) {
		  if (weather.WindSpeed < sysGlobal->shutdown_wind_speed) {
			if (!SysTimerActive[SYSTMR_WIND_SPEED_CLEAR]) {
			  SysTimer[SYSTMR_WIND_SPEED_CLEAR].
			  NewTimerSecs(sysGlobal->shutdown_delay_ws * 60);
			  SysTimerActive[SYSTMR_WIND_SPEED_CLEAR] = TRUE;
			}

			if (SysTimer[SYSTMR_WIND_SPEED_CLEAR].Expired()) {
				SysTimerActive[SYSTMR_WIND_SPEED_CLEAR] = FALSE;
				G->shutdown_state &= ~SDC_WIND;
			}
		  } else {
			SysTimerActive[SYSTMR_WIND_SPEED_CLEAR] = FALSE;
		  }
		} else {
		  if (weather.WindSpeed >= sysGlobal->shutdown_wind_speed) {
				G->need_to_shutdown = TRUE;
				G->shutdown_state |= SDC_WIND;
		  }
		}

		// check the outside temperature
		if (G->shutdown_state & SDC_OUTTEMP) {
		  if ((weather.OutTemp > sysGlobal->shutdown_out_low_temp)
		       && (weather.OutTemp < sysGlobal->shutdown_out_high_temp))
		  {
			if (!SysTimerActive[SYSTMR_OUT_TEMP_CLEAR]) {
				SysTimer[SYSTMR_OUT_TEMP_CLEAR].
				NewTimerSecs(sysGlobal->shutdown_delay_ws * 60);
				SysTimerActive[SYSTMR_OUT_TEMP_CLEAR] = TRUE;
			}

			if (SysTimer[SYSTMR_OUT_TEMP_CLEAR].Expired()) {
				SysTimerActive[SYSTMR_OUT_TEMP_CLEAR] = FALSE;
				G->shutdown_state &= ~SDC_OUTTEMP;
			}
		  } else {
			SysTimerActive[SYSTMR_OUT_TEMP_CLEAR] = FALSE;
		  }
		} else {
		  if ((weather.OutTemp <= sysGlobal->shutdown_out_low_temp) ||
		      (weather.OutTemp >= sysGlobal->shutdown_out_high_temp)) {
				G->need_to_shutdown = TRUE;
				G->shutdown_state |= SDC_OUTTEMP;
		  }
		}

		// check the cabinet temperature
		if (G->shutdown_state & SDC_CABTEMP) {
		  if ((weather.CabTemp > sysGlobal->shutdown_int_low_temp)
		     && (weather.CabTemp < sysGlobal->shutdown_int_high_temp))
		  {
			if (!SysTimerActive[SYSTMR_CAB_TEMP_CLEAR]) {
				SysTimer[SYSTMR_CAB_TEMP_CLEAR].
				NewTimerSecs(sysGlobal->shutdown_delay_ws * 60);
				SysTimerActive[SYSTMR_CAB_TEMP_CLEAR] = TRUE;
			}

			if (SysTimer[SYSTMR_CAB_TEMP_CLEAR].Expired()) {
				SysTimerActive[SYSTMR_CAB_TEMP_CLEAR] = FALSE;
				G->shutdown_state &= ~SDC_CABTEMP;
			}
		  } else {
			 SysTimerActive[SYSTMR_CAB_TEMP_CLEAR] = FALSE;
		  }
		} else {
		  if ((weather.CabTemp <= sysGlobal->shutdown_int_low_temp) ||
		      (weather.CabTemp >= sysGlobal->shutdown_int_high_temp)) {
				G->need_to_shutdown = TRUE;
				G->shutdown_state |= SDC_CABTEMP;
		  }
		}

		// shutdown if necessary
		if (G->need_to_shutdown)
			do_shutdown();
		#endif
	}

BOOL watchdog_ok(unsigned& status)
	{
		status = TCSERR_OK;

		#ifdef no_hardware
		return TRUE;
		#else
		if (!sysGlobal->watchdog_installed)
			return TRUE;

		// if the watchdog sense is high, return an error
		if (ocs_watchdog_active()) {
			status = TCSERR_WDOGTOUT;
			return FALSE;
		} else
			return TRUE;
		#endif
	}

#ifdef SOCKET
#ifdef __cplusplus
        extern "C" {
#endif
int read_udp(int lsock, char *line, int maxlen);
void sock_close(int);
#ifdef __cplusplus
        }
#endif
#endif
void check_35m()
{
#ifdef SOCKET
        static int sock, init=FALSE;
        int len;
        char line[80];
#endif

	// check the status of the 3.5m dome
	  if (!SysTimer[SYSTMR_35M_CHECK].Expired()) return;

	  //reset timer
	  SysTimer[SYSTMR_35M_CHECK].NewTimerSecs(sysGlobal->a35m_check_frequency);
	  BOOL a35m_closed = FALSE;
	  BOOL a35m_open = FALSE;
	  BOOL a35m_shutdown = FALSE;

	  // First see if 3.5m dome is actively showing closed
#ifdef SOCKET
          if (!init) {
            sprintf(outbuf,"trying to setup udp: \n");
            writeline(outbuf,1);
            setup_server(&sock, -19999);
            sprintf(outbuf,"udp socket: %d\n",sock);
            writeline(outbuf,1);
            if (sock>0) init = TRUE;
          } 
          line[0] = 0;
          len=read_udp(sock,line,80);
          if (strlen(line)>0) {
            sprintf(outbuf,"read_udp: %d %s\n",len,line);
            writeline(outbuf,1);
            if (strncmp(line,"shutters=0",10) == 0) {
              a35m_closed = TRUE;
            } 
            else if (strncmp(line,"shutters=40",11) == 0) {
            //else if (strncmp(line,"shutters=1",10) == 0) {
              a35m_open = TRUE;
            } else if (strncmp(line,"command=",8) == 0) {
              nudp = len-8;
              strncpy(udp_command,line+8,nudp);
              udp_command[nudp]=13;
              udp_command[++nudp]=0;
            }
          }
#else
	  FILE *fp;
	  if ((fp=fopen(sysGlobal->a35m_check_closed,"r")) != NULL) {
		    a35m_closed = TRUE;
		    fclose(fp);
	  }
#endif
          if (!G->check_35m_closed) return;

	  // Check to see if we are already shut down by 3.5m condition
	  if (G->shutdown_state & SDC_35M_CLOSED) {
	    if (!a35m_closed) {
	      // If 3.5m is closed, clear the condition
	      G->shutdown_state &= ~SDC_35M_CLOSED;
	    }
	  } 
	  if (a35m_closed) {
	    // We need to shut down.
	    G->need_to_shutdown = TRUE;
	    G->shutdown_state |= SDC_35M_CLOSED;
	  }

#ifndef SOCKET
	  // Now check to see if 3.5m dome is actively showing open
	  if ((fp=fopen(sysGlobal->a35m_check_opened,"r")) != NULL) {
		    a35m_open = TRUE;
		    fclose(fp);
	  }
#endif

	  // If we see open file, deactive the open check timer
	  if (a35m_open) {
	     SysTimerActive[SYSTMR_35M_OPEN] = FALSE;
	  } else {
	     // If we don't see the open file, start the open check timer if
	     //   it is not already active. If it is active, check to see
	     //   if it has expired. If so, we don't know if
	     //   3.5m is open or closed, so we'd better close
	     if (!SysTimerActive[SYSTMR_35M_OPEN]) {
		SysTimer[SYSTMR_35M_OPEN].NewTimerSecs(600);
		SysTimerActive[SYSTMR_35M_OPEN] = TRUE;
	     } else if (SysTimer[SYSTMR_35M_OPEN].Expired()) {
		SysTimerActive[SYSTMR_35M_OPEN] = FALSE;
		a35m_shutdown = TRUE;
	     }
	  }

	  // Check to see if we are already shut down by 3.5m condition
	  if (G->shutdown_state & SDC_35M_NOT_OPEN) {
	    if (a35m_open) {
	      // If 3.5m isn't closed, clear the condition
	      G->shutdown_state &= ~SDC_35M_NOT_OPEN;
	    }
	  } 
	  if (a35m_shutdown) {
	    // We need to shut down.
	    G->need_to_shutdown = TRUE;
	    G->shutdown_state |= SDC_35M_NOT_OPEN;
	  }

	  // shutdown if necessary
	  if (G->need_to_shutdown) do_shutdown();
}

//------------------------------------------------------------------
//      Name.......:    check_ups
//
//      Purpose....:    see if ups is ok
//
//      Input......:    none
//
//      Output.....:    none
//
//------------------------------------------------------------------
void check_ups()
{
  BOOL alive = FALSE;
  BOOL shutdown = FALSE;
  UPSPACK ups;

  // get ups status
  ups.line_fail = FALSE;
  ups.battery_low = FALSE;
  if (sysGlobal->ups_installed) ocs_ups_status(ups);

  // If OK, deactivate the check timer
  if (!ups.line_fail) {
      SysTimerActive[SYSTMR_POWER_LOST] = FALSE;
  } else {
      // If we don't see power, start the check timer if
      //   it is not already active. If it is active, check to see
      //   if it has expired. If so, we don't have power, so
      //   we'd better close
      if (!SysTimerActive[SYSTMR_POWER_LOST]) {
	SysTimer[SYSTMR_POWER_LOST].NewTimerSecs(SYSTMR_POWER_LOST_INC);
	SysTimerActive[SYSTMR_POWER_LOST] = TRUE;
      } else if (SysTimer[SYSTMR_POWER_LOST].Expired()) {
	SysTimerActive[SYSTMR_POWER_LOST] = FALSE;
	shutdown = TRUE;
      }
  }

  // Check to see if we are already shut down by network condition
  if (G->shutdown_state & SDC_UPS_LINE) {
      if (!ups.line_fail) {
	      // If power is alive, clear the condition
	      G->shutdown_state &= ~SDC_UPS_LINE;
      }
  } 
  if (shutdown) {
      // We need to shut down.
      G->need_to_shutdown = TRUE;
      G->shutdown_state |= SDC_UPS_LINE;
  }

  // shutdown if necessary
  if (G->need_to_shutdown) do_shutdown();

}
//------------------------------------------------------------------
//      Name.......:    check_network
//
//      Purpose....:    see if network aliveness file is still there
//
//      Input......:    none
//
//      Output.....:    none
//
//------------------------------------------------------------------
char *netfile = "e:\\tocc\\alive";

void check_network()
{
  FILE *fp;

  BOOL alive = FALSE;
  BOOL shutdown = FALSE;

  // Now check to see if network is alive
  if ((fp=fopen(netfile,"r")) != NULL) {
      alive = TRUE;
      fclose(fp);
  }

  // If we see alive file, deactive the network check timer
  if (alive) {
      SysTimerActive[SYSTMR_NETWORK_LOST] = FALSE;
      remove(netfile);
  } else {
      // If we don't see the alive file, start the network check timer if
      //   it is not already active. If it is active, check to see
      //   if it has expired. If so, we don't see the network, so
      //   we'd better close
      if (!SysTimerActive[SYSTMR_NETWORK_LOST]) {
	SysTimer[SYSTMR_NETWORK_LOST].NewTimerSecs(SYSTMR_NETWORK_LOST_INC);
	SysTimerActive[SYSTMR_NETWORK_LOST] = TRUE;
      } else if (SysTimer[SYSTMR_NETWORK_LOST].Expired()) {
	SysTimerActive[SYSTMR_NETWORK_LOST] = FALSE;
	shutdown = TRUE;
      }
  }

  // Check to see if we are already shut down by network condition
  if (G->shutdown_state & SDC_NO_NETWORK) {
      if (alive) {
	      // If network is alive, clear the condition
	      G->shutdown_state &= ~SDC_NO_NETWORK;
      }
  } 
  if (shutdown) {
      // We need to shut down.
      G->need_to_shutdown = TRUE;
      G->shutdown_state |= SDC_NO_NETWORK;
  }

  // shutdown if necessary
  if (G->need_to_shutdown) do_shutdown();

}
void reset_shutdown_timers()
{ 
  SysTimer[SYSTMR_NETWORK_LOST].NewTimerSecs(SYSTMR_NETWORK_LOST_INC);
  SysTimer[SYSTMR_35M_OPEN].NewTimerSecs(600);
}
/********************************* EOF **************************************/

