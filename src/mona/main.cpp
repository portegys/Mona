// For conditions of distribution and use, see copyright notice in mona.hpp

/*
 * Mona main interactive driver.
 *
 * Usage: mona [<input timeout (secs)>]
 *
 * Interaction is through standard input/output.
 * Can use with common/socker.cpp to create a simple network service.
 *
 * Set the primary parameters:
 * parameters: <number of sensors> <number of responses> <number of needs>
 *
 * This also causes the value of MAX_MEDIATOR_LEVEL to be written
 * to allow the optional setting of the event timeouts as follows:
 *
 * Add sensor mode:
 * add_sensor_mode: <sensor mask values> <sensor resolution>|"default_resolution"
 *
 * Set the number of effect event intervals for a mediator level:
 * set_intervals: <mediator level> <number of intervals>
 * The initial values will be set to 1.
 *
 * Set individual values within the interval arrays:
 * interval: <mediator level> <interval number> <interval value> <interval weight>
 *
 * Set maximum learning effect interval:
 * max_learn_interval: <mediator level> <interval value>
 *
 * Note: setting the parameters and intervals must be completed before
 * running cycles.
 *
 * Set the need values:
 * need: <need number> <need value>
 *
 * Set goal value for sensory input:
 * goal: <need number> <sensor values> <response (value or "null")> <goal value>
 * or
 * goal: [m]odal <need number> <sensor values> <sensor mode>
 * <response (value or "null")> <goal value>
 *
 * Run a sensory/response cycle:
 * cycle: <sensor values>
 * (output:) <response>
 *
 * Need changes may be interspersed between cycles.
 *
 * To override response (for training):
 * response: <override (value or "null")>
 *
 * To clear working memory:
 * erase: stm
 *
 * To reset long term memory:
 * erase: ltm
 *
 * To load from a file:
 * file: load <file name>
 *
 * To save to a file:
 * file: save <file name>
 *
 * To dump neural network to log:
 * dump
 *
 * To quit:
 * quit
 *
 * For help:
 * help
 *
 */

#include "mona.hpp"
#ifdef WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <ctype.h>
#endif

// Commands:
#define SET_PARAMETERS                            0
#define ADD_SENSOR_MODE                           1
#define SET_NUM_EFFECT_EVENT_INTERVALS            2
#define SET_EFFECT_EVENT_INTERVAL                 3
#define SET_MAX_LEARNING_EFFECT_EVENT_INTERVAL    4
#define SET_NEED                                  5
#define SET_GOAL                                  6
#define CYCLE                                     7
#define OVERRIDE_RESPONSE                         8
#define ERASE                                     9
#define FILEIO                                    10
#define LOG                                       11
#define DUMP                                      12
#define HELP                                      13
#define QUIT                                      14
#define UNKNOWN                                   15

// Input timeout.
int timeout = -1;

// Logging.
FILE *logfp = NULL;

#ifdef WIN32
UINT_PTR timer;

// Exit on stdin timeout.
VOID CALLBACK alrm_exit(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
   exit(1);
}


#endif

// Input error.
void inputError(char *msg)
{
   fprintf(stderr, "%s\n", msg);
   fflush(stderr);
   if (logfp != NULL)
   {
      fprintf(logfp, "%s\n", msg);
      fflush(logfp);
   }
}


// Get command
int getCommand()
{
   char command[50];

#ifdef WIN32
   if (timeout > 0)
   {
      timer = SetTimer(0, 0, timeout * 1000, (TIMERPROC)alrm_exit);
   }
#else
   if (timeout > 0)
   {
      alarm(timeout);
   }
#endif
   if (scanf("%49s", command) != 1)
   {
      inputError((char *)"Error reading command");
      exit(1);
   }
#ifdef WIN32
   if (timeout > 0)
   {
      KillTimer(0, timer);
   }
#else
   if (timeout > 0)
   {
      alarm(0);
   }
#endif
   switch (tolower(command[0]))
   {
   case 'p':
      return(SET_PARAMETERS);

   case 'a':
      return(ADD_SENSOR_MODE);

   case 's':
      return(SET_NUM_EFFECT_EVENT_INTERVALS);

   case 'i':
      return(SET_EFFECT_EVENT_INTERVAL);

   case 'm':
      return(SET_MAX_LEARNING_EFFECT_EVENT_INTERVAL);

   case 'n':
      return(SET_NEED);

   case 'g':
      return(SET_GOAL);

   case 'c':
      return(CYCLE);

   case 'r':
      return(OVERRIDE_RESPONSE);

   case 'e':
      return(ERASE);

   case 'f':
      return(FILEIO);

   case 'l':
      return(LOG);

   case 'd':
      return(DUMP);

   case 'h':
      return(HELP);

   case 'q':
      return(QUIT);

   default:
      return(UNKNOWN);
   }
}


int main(int argc, char *argv[])
{
   int    i, j, n, r;
   double w;
   int    numSensors, numResponses, numNeeds;

   vector<bool>         sensorMask;
   float                sensorResolution;
   Mona                 *mona;
   vector<Mona::SENSOR> sensors;
   int        response;
   Mona::NEED need;
   int        mode;
   bool       modal;
   char       buf[50];

   // Get optional timeout.
   if (argc == 2)
   {
      timeout = atoi(argv[1]);
   }
   if ((timeout < -1) || (timeout == 0) || (argc > 2))
   {
      fprintf(stderr, "Usage: %s [<input timeout (secs)>]\n", argv[0]);
      exit(1);
   }

   // Command loop.
   mona  = NULL;
   logfp = NULL;
   bool done = false;
   while (!done)
   {
      switch (getCommand())
      {
      case SET_PARAMETERS:
         if (scanf("%d %d %d", &numSensors, &numResponses, &numNeeds) != 3)
         {
            inputError((char *)"Error reading parameters");
            exit(1);
         }
         if (logfp != NULL)
         {
            fprintf(logfp, "parameters: %d %d %d\n", numSensors,
                    numResponses, numNeeds);
            fflush(logfp);
         }
         if (numSensors <= 0)
         {
            fprintf(stderr, "Number of sensors must be > 0\n");
            fflush(stderr);
            exit(1);
         }
         if (numResponses <= 0)
         {
            fprintf(stderr, "Number of responses must be > 0\n");
            fflush(stderr);
            exit(1);
         }
         if (numNeeds <= 0)
         {
            fprintf(stderr, "Number of needs must be > 0\n");
            fflush(stderr);
            exit(1);
         }

         // Create mona.
         if (mona != NULL)
         {
            delete mona;
         }
         mona = new Mona(numSensors, numResponses, numNeeds);
         if (mona == NULL)
         {
            fprintf(stderr, "Cannot create mona\n");
            fflush(stderr);
            exit(1);
         }
         sensors.resize(numSensors);

         // Write MAX_MEDIATOR_LEVEL parameter value to allow
         // subsequent settings of event intervals.
         printf("MAX_MEDIATOR_LEVEL=%d\n", mona->MAX_MEDIATOR_LEVEL);
         fflush(stdout);

         break;

      case ADD_SENSOR_MODE:
         if (logfp != NULL)
         {
            fprintf(logfp, "add sensor mode:");
         }
         if (mona == NULL)
         {
            fprintf(logfp, "\n");
            fflush(logfp);
            break;
         }
         fprintf(logfp, " mask=");
         sensorMask.clear();
         for (i = 0; i < numSensors; i++)
         {
            if (scanf("%d", &j) != 1)
            {
               inputError((char *)"Error reading sensor mask");
               exit(1);
            }
            if (j == 1)
            {
               sensorMask.push_back(true);
               if (logfp != NULL)
               {
                  fprintf(logfp, " 1");
               }
            }
            else
            {
               sensorMask.push_back(false);
               if (logfp != NULL)
               {
                  fprintf(logfp, " 0");
               }
            }
         }
         if (scanf("%49s", buf) != 1)
         {
            inputError((char *)"Error reading sensor resolution");
            exit(1);
         }
         if (buf[0] == 'd')
         {
            mona->addSensorMode(sensorMask);
            if (logfp != NULL)
            {
               fprintf(logfp, " resolution=default\n");
               fflush(logfp);
            }
         }
         else
         {
            sensorResolution = (float)atof(buf);
            if (logfp != NULL)
            {
               fprintf(logfp, " resolution=%f\n", sensorResolution);
               fflush(logfp);
            }
            if (sensorResolution < 0.0f)
            {
               fprintf(stderr, "Sensor resolution must be >= 0.0\n");
               fflush(stderr);
               exit(1);
            }
            mona->addSensorMode(sensorMask, sensorResolution);
         }
         break;

      case SET_NUM_EFFECT_EVENT_INTERVALS:
         if (scanf("%d %d", &i, &n) != 2)
         {
            inputError((char *)"Error reading number of effect event intervals");
            exit(1);
         }
         if (logfp != NULL)
         {
            fprintf(logfp, "set number of effect event intervals: %d %d\n", i, n);
            fflush(logfp);
         }
         if (mona == NULL)
         {
            break;
         }
         if ((i < 0) || (i > mona->MAX_MEDIATOR_LEVEL))
         {
            fprintf(stderr, "Invalid mediator level\n");
            fflush(stderr);
            exit(1);
         }
         if ((n <= 0) || (n > 100))
         {
            fprintf(stderr, "Invalid number of effect event intervals\n");
            fflush(stderr);
            exit(1);
         }
         mona->effectEventIntervals[i].resize(n);
         for (j = 0; j < n; j++)
         {
            mona->effectEventIntervals[i][j] = (int)(pow(2.0, i));
         }
         mona->initEffectEventIntervalWeights();
         break;

      case SET_EFFECT_EVENT_INTERVAL:
         if (scanf("%d %d %d %49s", &i, &j, &n, buf) != 4)
         {
            inputError((char *)"Error reading effect event intervals");
            exit(1);
         }
         w = atof(buf);
         if (logfp != NULL)
         {
            fprintf(logfp, "effect event interval: %d %d %d %f\n", i, j, n, w);
            fflush(logfp);
         }
         if (mona == NULL)
         {
            break;
         }
         if ((i < 0) || (i > mona->MAX_MEDIATOR_LEVEL))
         {
            fprintf(stderr, "Invalid mediator level\n");
            fflush(stderr);
            exit(1);
         }
         if ((j < 0) || (j >= (int)mona->effectEventIntervals[i].size()))
         {
            fprintf(stderr, "Invalid effect event interval\n");
            fflush(stderr);
            exit(1);
         }
         if (n < 0)
         {
            fprintf(stderr, "Invalid effect event interval value\n");
            fflush(stderr);
            exit(1);
         }
         if (w < 0.0)
         {
            fprintf(stderr, "Invalid effect event interval weight\n");
            fflush(stderr);
            exit(1);
         }
         mona->effectEventIntervals[i][j]       = n;
         mona->effectEventIntervalWeights[i][j] = w;
         break;

      case SET_MAX_LEARNING_EFFECT_EVENT_INTERVAL:
         if (scanf("%d %d", &i, &n) != 2)
         {
            inputError((char *)"Error reading maximum learning effect event intervals");
            exit(1);
         }
         if (logfp != NULL)
         {
            fprintf(logfp, "maximum learning effect event interval: %d %d\n", i, n);
            fflush(logfp);
         }
         if (mona == NULL)
         {
            break;
         }
         if ((i < 0) || (i > mona->MAX_MEDIATOR_LEVEL))
         {
            fprintf(stderr, "Invalid mediator level\n");
            fflush(stderr);
            exit(1);
         }
         if (n < 0)
         {
            fprintf(stderr, "Invalid maximum learning effect event interval value\n");
            fflush(stderr);
            exit(1);
         }
         mona->maxLearningEffectEventIntervals[i] = n;
         break;

      case SET_NEED:
         if (scanf("%d %49s", &i, buf) != 2)
         {
            inputError((char *)"Error reading set need command");
            exit(1);
         }
         need = atof(buf);
         if (logfp != NULL)
         {
            fprintf(logfp, "need: %d %f\n", i, need);
            fflush(logfp);
         }
         if (mona == NULL)
         {
            break;
         }
         if ((i < 0) || (i >= numNeeds))
         {
            fprintf(stderr, "Invalid need\n");
            fflush(stderr);
            exit(1);
         }
         mona->setNeed(i, need);
         break;

      case SET_GOAL:
         if (scanf("%49s", buf) != 1)
         {
            inputError((char *)"Error reading set goal command");
            exit(1);
         }
         if (tolower(buf[0]) == 'm')
         {
            modal = true;
            if (scanf("%d", &i) != 1)
            {
               inputError((char *)"Error reading goal number");
               exit(1);
            }
            if (logfp != NULL)
            {
               fprintf(logfp, "goal: modal %d ", i);
            }
         }
         else
         {
            modal = false;
            mode  = 0;
            i     = atoi(buf);
            if (logfp != NULL)
            {
               fprintf(logfp, "goal: %d ", i);
            }
         }
         for (j = 0; j < numSensors; j++)
         {
            if (scanf("%49s", buf) != 1)
            {
               inputError((char *)"Error reading goal sensor value");
               exit(1);
            }
            sensors[j] = (Mona::SENSOR)atof(buf);
            if (logfp != NULL)
            {
               fprintf(logfp, "%s ", buf);
            }
         }
         if (modal)
         {
            if (scanf("%d", &mode) != 1)
            {
               inputError((char *)"Error reading goal sensor mode");
               exit(1);
            }
            if (logfp != NULL)
            {
               fprintf(logfp, "%d ", mode);
            }
         }
         if (scanf("%49s", buf) != 1)
         {
            inputError((char *)"Error reading goal response");
            exit(1);
         }
         if (strcmp(buf, "null") == 0)
         {
            r = Mona::NULL_RESPONSE;
            if (logfp != NULL)
            {
               fprintf(logfp, "null ");
            }
         }
         else
         {
            r = atoi(buf);
            if (logfp != NULL)
            {
               fprintf(logfp, "%d ", r);
            }
         }
         if (scanf("%49s", buf) != 1)
         {
            inputError((char *)"Error reading goal value");
            exit(1);
         }
         need = atof(buf);
         if (logfp != NULL)
         {
            fprintf(logfp, "%s\n", buf);
            fflush(logfp);
         }
         if (mona == NULL)
         {
            break;
         }
         if ((i < 0) || (i >= numNeeds))
         {
            fprintf(stderr, "Invalid need\n");
            fflush(stderr);
            exit(1);
         }
         if (r < 0)
         {
            fprintf(stderr, "Invalid response\n");
            fflush(stderr);
            exit(1);
         }
         if ((r >= numResponses) && (r != Mona::NULL_RESPONSE))
         {
            fprintf(stderr, "Invalid response\n");
            fflush(stderr);
            exit(1);
         }
         mona->addGoal(i, sensors, mode, r, need);
         break;

      case CYCLE:
         if (logfp != NULL)
         {
            fprintf(logfp, "cycle: ");
         }
         if (mona == NULL)
         {
            break;
         }
         for (i = 0; i < numSensors; i++)
         {
            if (scanf("%49s", buf) != 1)
            {
               inputError((char *)"Error reading cycle sensor value");
               exit(1);
            }
            sensors[i] = (float)atof(buf);
            if (logfp != NULL)
            {
               fprintf(logfp, "%f ", sensors[i]);
            }
         }
         if (logfp != NULL)
         {
            fprintf(logfp, "\n");
            fflush(logfp);
         }
         response = mona->cycle(sensors);
         mona->responseOverride          = Mona::NULL_RESPONSE;
         mona->responseOverridePotential = -1.0;
         printf("%d\n", response);
         fflush(stdout);
         if (logfp != NULL)
         {
            fprintf(logfp, "response=%d\n", response);
            fflush(logfp);
         }
         break;

      case OVERRIDE_RESPONSE:
         if (scanf("%49s", buf) != 1)
         {
            inputError((char *)"Error reading override response");
            exit(1);
         }
         if (strcmp(buf, "null") == 0)
         {
            response = Mona::NULL_RESPONSE;
            if (logfp != NULL)
            {
               fprintf(logfp, "override response=null\n");
               fflush(logfp);
            }
         }
         else
         {
            response = atoi(buf);
            if (logfp != NULL)
            {
               fprintf(logfp, "override response=%d\n", response);
               fflush(logfp);
            }
         }
         if (mona == NULL)
         {
            break;
         }
         if (response < 0)
         {
            fprintf(stderr, "Invalid response\n");
            fflush(stderr);
            exit(1);
         }
         if ((response >= numResponses) && (response != Mona::NULL_RESPONSE))
         {
            fprintf(stderr, "Invalid response\n");
            fflush(stderr);
            exit(1);
         }
         mona->responseOverride = response;
         break;

      case ERASE:
         if (scanf("%49s", buf) != 1)
         {
            inputError((char *)"Error reading erase command");
            exit(1);
         }
         if (logfp != NULL)
         {
            fprintf(logfp, "erase: %s\n", buf);
            fflush(logfp);
         }
         if (mona == NULL)
         {
            break;
         }
         mona->clearWorkingMemory();
         if (strcmp(buf, "ltm") == 0)
         {
            mona->clearLongTermMemory();
         }
         break;

      case FILEIO:
         if (scanf("%49s", buf) != 1)
         {
            inputError((char *)"Error reading file command");
            exit(1);
         }
         if (strcmp(buf, "load") == 0)
         {
            if (scanf("%49s", buf) != 1)
            {
               inputError((char *)"Error reading file load command");
               exit(1);
            }
            if (logfp != NULL)
            {
               fprintf(logfp, "file: load %s\n", buf);
               fflush(logfp);
            }
            if (mona == NULL)
            {
               mona = new Mona();
               if (mona == NULL)
               {
                  fprintf(stderr, "Cannot create mona\n");
                  fflush(stderr);
                  exit(1);
               }
            }
            mona->load(buf);
         }
         else if (strcmp(buf, "save") == 0)
         {
            if (scanf("%49s", buf) != 1)
            {
               inputError((char *)"Error reading file save command");
               exit(1);
            }
            if (logfp != NULL)
            {
               fprintf(logfp, "file: save %s\n", buf);
               fflush(logfp);
            }
            if (mona == NULL)
            {
               break;
            }
            mona->save(buf);
         }
         else
         {
            fprintf(stderr, "Invalid file command\n");
            fflush(stderr);
            exit(1);
         }
         break;

      case LOG:
         if (scanf("%49s", buf) != 1)
         {
            inputError((char *)"Error reading log command");
            exit(1);
         }
         if (strcmp(buf, "on") == 0)
         {
            if (logfp == NULL)
            {
#ifdef WIN32
               sprintf(buf, "mona%d.log", _getpid());
#else
               sprintf(buf, "/tmp/mona%d.log", getpid());
#endif
               if ((logfp = fopen(buf, "w")) == NULL)
               {
                  fprintf(stderr, "Cannot open log file %s\n", buf);
                  fflush(stderr);
               }
               else
               {
                  fprintf(logfp, "logging to file %s\n", buf);
                  fflush(logfp);
               }
            }
         }
         else
         {
            if (logfp != NULL)
            {
               fprintf(logfp, "logging off\n");
               fclose(logfp);
               logfp = NULL;
            }
         }
         break;

      case DUMP:
         if ((logfp != NULL) && (mona != NULL))
         {
            mona->print(logfp);
         }
         break;

      case HELP:
         printf("Commands:\n");
         printf("[p]arameters: <number of sensors> <number of responses> <number of needs>\n");
         printf("[a]dd_sensor_mode: <sensor mask values> <sensor resolution>|\"default_resolution\"\n");
         printf("[s]et_intervals: <mediator level> <number of intervals>\n");
         printf("[i]nterval: <mediator level> <interval number> <interval value> <interval weight>\n");
         printf("[m]ax_learn_interval: <mediator level> <interval value>\n");
         printf("[n]eed: <need number> <need value>\n");
         printf("[g]oal: <need number> <sensor values> <response (value or \"null\")> <goal value>\n");
         printf("or\n");
         printf("[g]oal: [m]odal <need number> <sensor values> <sensor mode> <response (value or \"null\")> <goal value>\n");
         printf("[c]ycle: <sensor values> (triggers response output)\n");
         printf("[r]esponse: <override (value or \"null\")>\n");
         printf("[e]rase: stm (short term memory) | ltm (long term memory)\n");
         printf("[f]ile: load <file name>\n");
         printf("[f]ile: save <file name>\n");
         printf("[l]ogging on | off");
#ifdef WIN32
         printf(" (file: mona%d.log)\n", _getpid());
#else
         printf(" (file: /tmp/mona%d.log)\n", getpid());
#endif
         printf("[d]ump (neural network to log)\n");
         printf("[h]elp\n");
         printf("[q]uit\n");
         fflush(stdout);
         if (logfp != NULL)
         {
            fprintf(logfp, "help\n");
            fprintf(logfp, "Commands:\n");
            fprintf(logfp, "[p]arameters: <number of sensors> <number of responses> <number of needs>\n");
            fprintf(logfp, "[a]dd_sensor_mode: <sensor mask values> <sensor resolution>|\"default_resolution\"\n");
            fprintf(logfp, "[s]et_intervals: <mediator level> <number of intervals>\n");
            fprintf(logfp, "[i]nterval: <mediator level> <interval number> <interval value> <interval weight>\n");
            fprintf(logfp, "[m]ax_learn_interval: <mediator level> <interval value>\n");
            fprintf(logfp, "[n]eed: <need number> <need value>\n");
            fprintf(logfp, "[g]oal: <need number> <sensor values> <response (value or \"null\")> <goal value>\n");
            fprintf(logfp, "or\n");
            fprintf(logfp, "[g]oal: [m]odal <need number> <sensor values> <sensor mode> <response (value or \"null\")> <goal value>\n");
            fprintf(logfp, "[c]ycle: <sensor values> (triggers response output)\n");
            fprintf(logfp, "[r]esponse: <override (value or \"null\")>\n");
            fprintf(logfp, "[e]rase: stm (short term memory) | ltm (long term memory)\n");
            fprintf(logfp, "[f]ile: load <file name>\n");
            fprintf(logfp, "[f]ile: save <file name>\n");
            fprintf(logfp, "[l]ogging on | off");
#ifdef WIN32
            fprintf(logfp, " (to file: mona%d.log)\n", _getpid());
#else
            fprintf(logfp, " (to file: /tmp/mona%d.log)\n", getpid());
#endif
            fprintf(logfp, "[d]ump (neural network to log)\n");
            fprintf(logfp, "[h]elp\n");
            fprintf(logfp, "[q]uit\n");
            fflush(logfp);
         }
         break;

      case QUIT:
         if (logfp != NULL)
         {
            fprintf(logfp, "quit\n");
            fclose(logfp);
         }
         done = true;
         break;

      default:
         printf("Unknown command\n");
         fflush(stdout);
         if (logfp != NULL)
         {
            fprintf(logfp, "Unknown command\n");
            fflush(logfp);
         }
         break;
      }
   }

   if (mona != NULL)
   {
      delete mona;
   }
   return(0);
}
