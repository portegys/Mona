// Mona native JNI implementation.
// For conditions of distribution and use, see copyright notice in mona.hpp

#ifndef WIN32
typedef long long   __int64;
#endif

#include "mona_NativeFileDescriptor.h"
#include "mona_Mona.h"
#include "mona.hpp"

// Mona objects.
vector<Mona *> monaList;

/*
 * Class:     mona_Mona
 * Method:    createMona
 * Signature: (IIII)I
 */
JNIEXPORT jint JNICALL Java_mona_Mona_createMona(
   JNIEnv *env, jobject object, jint numSensors,
   jint numResponses, jint numNeeds, jint randomSeed)
{
   Mona *mona = new Mona(numSensors, numResponses,
                         numNeeds, (RANDOM)randomSeed);

   assert(mona);
   monaList.push_back(mona);
   return((jint)monaList.size() - 1);
}


/*
 * Class:     mona_Mona
 * Method:    createMonaWithParameters
 * Signature: ([Ljava/lang/String;[Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL Java_mona_Mona_createMonaWithParameters(JNIEnv *env, jobject object,
                                                               jobjectArray parameterKeys,
                                                               jobjectArray parameterValues)
{
   // Create mona without network initialization.
   Mona *mona = new Mona();

   assert(mona);

   int numSensors   = 0;
   int numResponses = 0;
   int numNeeds     = 0;
   vector<vector<bool> > sensorMasks;
   vector<float>         sensorResolutions;
   RANDOM                randomSeed = Mona::DEFAULT_RANDOM_SEED;

   jsize      jNumParms = env->GetArrayLength(parameterKeys);
   jstring    jParm, jVal;
   const char *parm, *val;
   for (int i = 0; i < jNumParms; i++)
   {
      jParm = (jstring)env->GetObjectArrayElement(parameterKeys, i);
      parm  = env->GetStringUTFChars(jParm, 0);
      assert(parm);

      if (strcmp(parm, "SENSOR_MODE") == 0)
      {
         jobjectArray jarrvals = (jobjectArray)env->GetObjectArrayElement(parameterValues, i);
         jsize        jNumvals = env->GetArrayLength(jarrvals);
         jstring      javal;
         const char   *aval;
         vector<bool> mask;
         float        resolution;
         for (int j = 0; j < jNumvals; j++)
         {
            javal = (jstring)env->GetObjectArrayElement(jarrvals, j);
            aval  = env->GetStringUTFChars(javal, 0);
            assert(aval);
            if (j < jNumvals - 1)
            {
               if (atoi(aval) == 1)
               {
                  mask.push_back(true);
               }
               else
               {
                  mask.push_back(false);
               }
            }
            else
            {
               resolution = (float)atof(aval);
            }
            env->ReleaseStringUTFChars(javal, aval);
         }
         sensorMasks.push_back(mask);
         sensorResolutions.push_back(resolution);
         env->ReleaseStringUTFChars(jParm, parm);

         continue;
      }

      jVal = (jstring)env->GetObjectArrayElement(parameterValues, i);
      val  = env->GetStringUTFChars(jVal, 0);
      assert(val);

      if (strcmp(parm, "NUM_SENSORS") == 0)
      {
         numSensors = atoi(val);
      }
      else if (strcmp(parm, "NUM_RESPONSES") == 0)
      {
         numResponses = atoi(val);
      }
      else if (strcmp(parm, "NUM_NEEDS") == 0)
      {
         numNeeds = atoi(val);
      }
      else if (strcmp(parm, "RANDOM_SEED") == 0)
      {
         randomSeed = atoi(val);
      }
      else if (strcmp(parm, "MIN_ENABLEMENT") == 0)
      {
         mona->MIN_ENABLEMENT = atof(val);
      }
      else if (strcmp(parm, "INITIAL_ENABLEMENT") == 0)
      {
         mona->INITIAL_ENABLEMENT = atof(val);
      }
      else if (strcmp(parm, "DRIVE_ATTENUATION") == 0)
      {
         mona->DRIVE_ATTENUATION = atof(val);
      }
      else if (strcmp(parm, "LEARNING_DECREASE_VELOCITY") == 0)
      {
         mona->LEARNING_DECREASE_VELOCITY = atof(val);
      }
      else if (strcmp(parm, "LEARNING_INCREASE_VELOCITY") == 0)
      {
         mona->LEARNING_INCREASE_VELOCITY = (float)atof(val);
      }
      else if (strcmp(parm, "FIRING_STRENGTH_LEARNING_DAMPER") == 0)
      {
         mona->FIRING_STRENGTH_LEARNING_DAMPER = atof(val);
      }
      else if (strcmp(parm, "UTILITY_ASYMPTOTE") == 0)
      {
         mona->UTILITY_ASYMPTOTE = atof(val);
      }
      else if (strcmp(parm, "RESPONSE_RANDOMNESS") == 0)
      {
         mona->RESPONSE_RANDOMNESS = atof(val);
      }
      else if (strcmp(parm, "DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL") == 0)
      {
         mona->DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL = atoi(val);
         mona->initMaxLearningEffectEventIntervals();
      }
      else if (strcmp(parm, "DEFAULT_NUM_EFFECT_EVENT_INTERVALS") == 0)
      {
         mona->DEFAULT_NUM_EFFECT_EVENT_INTERVALS = atoi(val);
         mona->initEffectEventIntervals();
      }
      else if (strcmp(parm, "MAX_MEDIATORS") == 0)
      {
         mona->MAX_MEDIATORS = atoi(val);
      }
      else if (strcmp(parm, "MAX_MEDIATOR_LEVEL") == 0)
      {
         mona->MAX_MEDIATOR_LEVEL = atoi(val);
         mona->initEffectEventIntervals();
         mona->initMaxLearningEffectEventIntervals();
      }
      else if (strcmp(parm, "MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL") == 0)
      {
         mona->MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL = atoi(val);
         mona->initMaxLearningEffectEventIntervals();
      }
      else if (strcmp(parm, "MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL") == 0)
      {
         mona->MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL = atoi(val);
      }
      else if (strcmp(parm, "SENSOR_RESOLUTION") == 0)
      {
         mona->SENSOR_RESOLUTION = (Mona::SENSOR)atof(val);
      }

      env->ReleaseStringUTFChars(jVal, val);
      env->ReleaseStringUTFChars(jParm, parm);
   }

   assert(numSensors > 0);
   assert(numResponses >= 0);
   assert(numNeeds > 0);

   // Initialize the network.
   mona->initNet(numSensors, numResponses, numNeeds, randomSeed);
   for (int i = 0; i < (int)sensorMasks.size(); i++)
   {
      mona->addSensorMode(sensorMasks[i], sensorResolutions[i]);
   }
   monaList.push_back(mona);
   return((jint)monaList.size() - 1);
}


/*
 * Class:     mona_Mona
 * Method:    addSensorMode
 * Signature: (I[Z)I
 */
JNIEXPORT jint JNICALL Java_mona_Mona_addSensorMode__I_3Z(
   JNIEnv *env, jobject object, jint reference, jbooleanArray mask)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      jboolean *jmask = env->GetBooleanArrayElements(mask, NULL);
      assert(jmask);
      vector<bool> m;
      for (int i = 0; i < mona->numSensors; i++)
      {
         if (jmask[i])
         {
            m.push_back(true);
         }
         else
         {
            m.push_back(false);
         }
      }
      env->ReleaseBooleanArrayElements(mask, jmask, 0);
      return(mona->addSensorMode(m));
   }
   else
   {
      return(-1);
   }
}


/*
 * Class:     mona_Mona
 * Method:    addSensorMode
 * Signature: (I[ZF)I
 */
JNIEXPORT jint JNICALL Java_mona_Mona_addSensorMode__I_3ZF(
   JNIEnv *env, jobject object, jint reference,
   jbooleanArray mask, jfloat resolution)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      jboolean *jmask = env->GetBooleanArrayElements(mask, NULL);
      assert(jmask);
      vector<bool> m;
      for (int i = 0; i < mona->numSensors; i++)
      {
         if (jmask[i])
         {
            m.push_back(true);
         }
         else
         {
            m.push_back(false);
         }
      }
      env->ReleaseBooleanArrayElements(mask, jmask, 0);
      return(mona->addSensorMode(m, (float)resolution));
   }
   else
   {
      return(-1);
   }
}


/*
 * Class:     mona_Mona
 * Method:    setEffectEventIntervals
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_mona_Mona_setEffectEventIntervals(
   JNIEnv *env, jobject object, jint reference,
   jint level, jint intervals)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      mona->effectEventIntervals[level].resize(intervals);
      for (int i = 0; i < intervals; i++)
      {
         mona->effectEventIntervals[level][i] = (int)(pow(2.0, level));
      }
      mona->initEffectEventIntervalWeights();
   }
}


/*
 * Class:     mona_Mona
 * Method:    setEffectEventInterval
 * Signature: (IIIIF)V
 */
JNIEXPORT void JNICALL Java_mona_Mona_setEffectEventInterval(
   JNIEnv *env, jobject object, jint reference,
   jint level, jint interval, jint value, jfloat weight)

{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      mona->effectEventIntervals[level][interval]       = value;
      mona->effectEventIntervalWeights[level][interval] = weight;
   }
}


/*
 * Class:     mona_Mona
 * Method:    dispose
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_mona_Mona_dispose(
   JNIEnv *env, jobject object, jint reference)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      delete mona;
      monaList[reference] = NULL;
   }
}


/*
 * Class:     mona_Mona
 * Method:    cycle
 * Signature: (I[F)I
 */
JNIEXPORT jint JNICALL Java_mona_Mona_cycle(
   JNIEnv *env, jobject object, jint reference, jfloatArray sensors)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      int    i;
      jfloat *jsensors = env->GetFloatArrayElements(sensors, NULL);
      assert(jsensors);
      vector<float> s;
      for (i = 0; i < mona->numSensors; i++)
      {
         s.push_back(jsensors[i]);
      }
      env->ReleaseFloatArrayElements(sensors, jsensors, 0);
      i = mona->cycle(s);
      return(i);
   }
   else
   {
      return(0);
   }
}


/*
 * Class:     mona_Mona
 * Method:    addResponse
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_mona_Mona_addResponse(JNIEnv *env, jobject object, jint reference)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      return(mona->addResponse());
   }
   else
   {
      return(-1);
   }
}


/*
 * Class:     mona_Mona
 * Method:    getResponsePotential
 * Signature: (II)D
 */
JNIEXPORT jdouble JNICALL Java_mona_Mona_getResponsePotential(
   JNIEnv *env, jobject object, jint reference, jint response)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      return(mona->getResponsePotential((Mona::RESPONSE)response));
   }
   else
   {
      return(0.0);
   }
}


/*
 * Class:     mona_Mona
 * Method:    overrideResponse
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_mona_Mona_overrideResponse(
   JNIEnv *env, jobject object, jint reference, jint response)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      mona->overrideResponse((Mona::RESPONSE)response);
   }
}


/*
 * Class:     mona_Mona
 * Method:    clearResponseOverride
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_mona_Mona_clearResponseOverride(
   JNIEnv *env, jobject object, jint reference)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      mona->clearResponseOverride();
   }
}


/*
 * Class:     mona_Mona
 * Method:    getNeed
 * Signature: (II)D
 */
JNIEXPORT jdouble JNICALL Java_mona_Mona_getNeed(
   JNIEnv *env, jobject object, jint reference, jint index)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      return(mona->getNeed(index));
   }
   else
   {
      return(0.0);
   }
}


/*
 * Class:     mona_Mona
 * Method:    setNeed
 * Signature: (IID)V
 */
JNIEXPORT void JNICALL Java_mona_Mona_setNeed(
   JNIEnv *env, jobject object, jint reference,
   jint index, jdouble value)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      mona->setNeed(index, value);
   }
}


/*
 * Class:     mona_Mona
 * Method:    addGoal
 * Signature: (II[FIID)V
 */
JNIEXPORT jint JNICALL Java_mona_Mona_addGoal__II_3FIID(
   JNIEnv *env, jobject object, jint reference,
   jint needIndex, jfloatArray sensors, jint sensorMode,
   jint response, jdouble goalValue)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      jfloat *jsensors = env->GetFloatArrayElements(sensors, NULL);
      assert(jsensors);
      vector<float> s;
      for (int i = 0; i < mona->numSensors; i++)
      {
         s.push_back(jsensors[i]);
      }
      env->ReleaseFloatArrayElements(sensors, jsensors, 0);
      return(mona->addGoal(needIndex, s, sensorMode, (Mona::RESPONSE)response, goalValue));
   }
   else
   {
      return(-1);
   }
}


/*
 * Class:     mona_Mona
 * Method:    addGoal
 * Signature: (II[FID)I
 */
JNIEXPORT jint JNICALL Java_mona_Mona_addGoal__II_3FID(JNIEnv *env, jobject object, jint reference, jint needIndex,
                                                       jfloatArray sensors, jint sensorMode, jdouble goalValue)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      jfloat *jsensors = env->GetFloatArrayElements(sensors, NULL);

      assert(jsensors);
      vector<float> s;
      for (int i = 0; i < mona->numSensors; i++)
      {
         s.push_back(jsensors[i]);
      }
      env->ReleaseFloatArrayElements(sensors, jsensors, 0);
      return(mona->addGoal(needIndex, s, sensorMode, goalValue));
   }
   else
   {
      return(-1);
   }
}


/*
 * Class:     mona_Mona
 * Method:    findGoal
 * Signature: (II[FII)I
 */
JNIEXPORT jint JNICALL Java_mona_Mona_findGoal__II_3FII(
   JNIEnv *env, jobject object, jint reference, jint needIndex,
   jfloatArray sensors, jint sensorMode, jint response)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      jfloat *jsensors = env->GetFloatArrayElements(sensors, NULL);
      assert(jsensors);
      vector<float> s;
      for (int i = 0; i < mona->numSensors; i++)
      {
         s.push_back(jsensors[i]);
      }
      env->ReleaseFloatArrayElements(sensors, jsensors, 0);
      if (response == -1)
      {
         return(mona->findGoal(needIndex, s, sensorMode));
      }
      else
      {
         return(mona->findGoal(needIndex, s, sensorMode, (Mona::RESPONSE)response));
      }
   }
   else
   {
      return(-1);
   }
}


/*
 * Class:     mona_Mona
 * Method:    findGoal
 * Signature: (II[FI)I
 */
JNIEXPORT jint JNICALL Java_mona_Mona_findGoal__II_3FI(
   JNIEnv *env, jobject object, jint reference, jint needIndex,
   jfloatArray sensors, jint sensorMode)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      jfloat *jsensors = env->GetFloatArrayElements(sensors, NULL);
      assert(jsensors);
      vector<float> s;
      for (int i = 0; i < mona->numSensors; i++)
      {
         s.push_back(jsensors[i]);
      }
      env->ReleaseFloatArrayElements(sensors, jsensors, 0);
      return(mona->findGoal(needIndex, s, sensorMode));
   }
   else
   {
      return(-1);
   }
}


/*
 * Class:     mona_Mona
 * Method:    getNumGoals
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_mona_Mona_getNumGoals(
   JNIEnv *env, jobject object, jint reference, jint needIndex)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      return(mona->getNumGoals(needIndex));
   }
   else
   {
      return(-1);
   }
}


/*
 * Class:     mona_Mona
 * Method:    isGoalEnabled
 * Signature: (III)Z
 */
JNIEXPORT jboolean JNICALL Java_mona_Mona_isGoalEnabled(
   JNIEnv *env, jobject object, jint reference,
   jint needIndex, jint goalIndex)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      return(mona->isGoalEnabled(needIndex, goalIndex));
   }
   else
   {
      return(false);
   }
}


/*
 * Class:     mona_Mona
 * Method:    enableGoal
 * Signature: (III)Z
 */
JNIEXPORT jboolean JNICALL Java_mona_Mona_enableGoal(
   JNIEnv *env, jobject object, jint reference,
   jint needIndex, jint goalIndex)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      return(mona->enableGoal(needIndex, goalIndex));
   }
   else
   {
      return(false);
   }
}


/*
 * Class:     mona_Mona
 * Method:    disableGoal
 * Signature: (III)Z
 */
JNIEXPORT jboolean JNICALL Java_mona_Mona_disableGoal(
   JNIEnv *env, jobject object, jint reference,
   jint needIndex, jint goalIndex)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      return(mona->disableGoal(needIndex, goalIndex));
   }
   else
   {
      return(false);
   }
}


/*
 * Class:     mona_Mona
 * Method:    removeGoal
 * Signature: (III)Z
 */
JNIEXPORT jboolean JNICALL Java_mona_Mona_removeGoal(
   JNIEnv *env, jobject object, jint reference,
   jint needIndex, jint goalIndex)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      return(mona->removeGoal(needIndex, goalIndex));
   }
   else
   {
      return(false);
   }
}


/*
 * Class:     mona_Mona
 * Method:    load
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_mona_Mona_load__ILjava_lang_String_2(
   JNIEnv *env, jobject object, jint reference, jstring filename)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      const char *str;
      str = env->GetStringUTFChars(filename, 0);
      assert(str);
      jboolean ret = mona->load((char *)str);
      env->ReleaseStringUTFChars(filename, str);
      return(ret);
   }
   else
   {
      return(false);
   }
}


/*
 * Class:     mona_Mona
 * Method:    load
 * Signature: (ILmona/NativeFileDescriptor;)Z
 */
JNIEXPORT jboolean JNICALL Java_mona_Mona_load__ILmona_NativeFileDescriptor_2(
   JNIEnv *env, jobject object, jint reference, jobject fdobj)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      // Get the file pointer.
      jclass   class_fdesc = env->GetObjectClass(fdobj);
      jfieldID field_fp    = env->GetFieldID(class_fdesc, "fp", "J");
      FILE     *fp         = (FILE *)env->GetLongField(fdobj, field_fp);

      // Load.
      if (fp != NULL)
      {
         jboolean ret = mona->load(fp);
         return(ret);
      }
   }
   return(false);
}


/*
 * Class:     mona_Mona
 * Method:    save
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_mona_Mona_save__ILjava_lang_String_2(
   JNIEnv *env, jobject object, jint reference, jstring filename)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      const char *str;
      str = env->GetStringUTFChars(filename, 0);
      assert(str);
      jboolean ret = mona->save((char *)str);
      env->ReleaseStringUTFChars(filename, str);
      return(ret);
   }
   else
   {
      return(false);
   }
}


/*
 * Class:     mona_Mona
 * Method:    save
 * Signature: (ILmona/NativeFileDescriptor;)Z
 */
JNIEXPORT jboolean JNICALL Java_mona_Mona_save__ILmona_NativeFileDescriptor_2(
   JNIEnv *env, jobject object, jint reference, jobject fdobj)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      // Get the file pointer.
      jclass   class_fdesc = env->GetObjectClass(fdobj);
      jfieldID field_fp    = env->GetFieldID(class_fdesc, "fp", "J");
      FILE     *fp         = (FILE *)env->GetLongField(fdobj, field_fp);

      // Save.
      if (fp != NULL)
      {
         jboolean ret = mona->save(fp);
         return(ret);
      }
   }
   return(false);
}


/*
 * Class:     mona_Mona
 * Method:    clearWorkingMemory
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_mona_Mona_clearWorkingMemory(
   JNIEnv *env, jobject object, jint reference)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      mona->clearWorkingMemory();
   }
}


/*
 * Class:     mona_Mona
 * Method:    clearLongTermMemory
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_mona_Mona_clearLongTermMemory(
   JNIEnv *env, jobject object, jint reference)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      mona->clearLongTermMemory();
   }
}


/*
 * Class:     mona_Mona
 * Method:    clear
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_mona_Mona_clear(
   JNIEnv *env, jobject object, jint reference)
{
   Mona *mona = monaList[reference];

   if (mona != NULL)
   {
      mona->clear();
   }
}


/*
 * Class:     mona_Mona
 * Method:    print_jni
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_mona_Mona_print_1jni(
   JNIEnv *env, jobject object, jint reference, jstring filename)
{
   Mona *mona = monaList[reference];

   if (mona == NULL)
   {
      return(false);
   }
   jboolean   ret = true;
   const char *str;
   str = env->GetStringUTFChars(filename, 0);
   if (strcmp(str, "stdout") == 0)
   {
      mona->print();
   }
   else
   {
      FILE *fp = fopen(str, "w");
      if (fp == NULL)
      {
         ret = false;
      }
      else
      {
         mona->print(fp);
         fclose(fp);
      }
   }
   env->ReleaseStringUTFChars(filename, str);
   return(ret);
}


/*
 * Class:     mona_NativeFileDescriptor
 * Method:    open
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_mona_NativeFileDescriptor_open(
   JNIEnv *env, jobject fdobj)
{
   jclass   class_fdesc;
   jfieldID field_filename;
   jstring  jfilename;
   char     *str_filename;
   jfieldID field_mode;
   jstring  jmode;
   char     *str_mode;
   jfieldID field_fp;
   FILE     *fp;
   jboolean ret = false;

   class_fdesc    = env->GetObjectClass(fdobj);
   field_filename = env->GetFieldID(class_fdesc, "filename", "Ljava/lang/String;");
   jfilename      = (jstring)env->GetObjectField(fdobj, field_filename);
   str_filename   = (char *)env->GetStringUTFChars(jfilename, NULL);
   field_mode     = env->GetFieldID(class_fdesc, "mode", "Ljava/lang/String;");
   jmode          = (jstring)env->GetObjectField(fdobj, field_mode);
   str_mode       = (char *)env->GetStringUTFChars(jmode, NULL);

   // Open and set unbuffered mode.
   if ((fp = fopen(str_filename, str_mode)) != NULL)
   {
      ret      = true;
      field_fp = env->GetFieldID(class_fdesc, "fp", "J");
      env->SetLongField(fdobj, field_fp, (jlong)fp);
      setbuf(fp, NULL);
   }
   env->ReleaseStringUTFChars(jfilename, str_filename);
   env->ReleaseStringUTFChars(jmode, str_mode);
   return(ret);
}


/*
 * Class:     mona_NativeFileDescriptor
 * Method:    close
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_mona_NativeFileDescriptor_close(
   JNIEnv *env, jobject fdobj)
{
   jclass   class_fdesc;
   jfieldID field_fp;
   FILE     *fp;

   class_fdesc = env->GetObjectClass(fdobj);
   field_fp    = env->GetFieldID(class_fdesc, "fp", "J");
   fp          = (FILE *)env->GetLongField(fdobj, field_fp);
   if ((fp != NULL) && (fclose(fp) == 0))
   {
      return(true);
   }
   else
   {
      return(false);
   }
}


/*
 * Class:     mona_NativeFileDescriptor
 * Method:    seek
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_mona_NativeFileDescriptor_seek(
   JNIEnv *env, jobject fdobj, jlong position)
{
   jclass   class_fdesc;
   jfieldID field_fp;
   FILE     *fp;

   class_fdesc = env->GetObjectClass(fdobj);
   field_fp    = env->GetFieldID(class_fdesc, "fp", "J");
   fp          = (FILE *)env->GetLongField(fdobj, field_fp);
   if ((fp != NULL) && (fseek(fp, (off_t)position, SEEK_SET) == 0))
   {
      return(true);
   }
   else
   {
      return(false);
   }
}


/*
 * Class:     mona_NativeFileDescriptor
 * Method:    tell
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_mona_NativeFileDescriptor_tell(
   JNIEnv *env, jobject fdobj)
{
   jclass   class_fdesc;
   jfieldID field_fp;
   FILE     *fp;

   class_fdesc = env->GetObjectClass(fdobj);
   field_fp    = env->GetFieldID(class_fdesc, "fp", "J");
   fp          = (FILE *)env->GetLongField(fdobj, field_fp);
   if (fp != NULL)
   {
      return(ftell(fp));
   }
   else
   {
      return(false);
   }
}
