/*
 * (C) Copyright 2015 Kurento (http://kurento.org/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "ResourceManager.hpp"
#include <gst/gst.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <KurentoException.hpp>
#include <MediaSet.hpp>

#ifdef __linux__
#include <sys/resource.h>
#elif defined(_WIN32)
#include <windows.h>
#include <tlhelp32.h>
#endif

#define GST_CAT_DEFAULT kurento_resource_manager
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoResourceManager"

namespace kurento
{

#ifndef _WIN32
static int maxOpenFiles = 0;
static int maxThreads = 0;

static int
get_int (std::string &str, char sep, int nToken)
{
  size_t start = str.find_first_not_of (sep), end;
  int count = 0;

  while (start != std::string::npos) {
    end = str.find (sep, start);

    if (count++ == nToken) {
      str[end] = '\0';
      return atoi (&str.c_str() [start]);
    }

    start = str.find_first_not_of (sep, end);
  }

  return 0;
}

static int
getNumberOfThreads ()
{
#ifdef __linux__
  std::string stat;
  std::ifstream stat_file ("/proc/self/stat");
  std::vector <std::string> tokens;

  std::getline (stat_file, stat);
  tokenize (stat, ' ', tokens);

  stat_file.close();

  return atoi (tokens[19].c_str () );
#elif defined (_WIN32)
  DWORD const id = GetCurrentProcessId ();
  HANDLE const snapshot = CreateToolhelp32Snapshot (TH32CS_SNAPALL, 0);
  PROCESSENTRY32 entry = { 0 };
  entry.dwSize = sizeof (entry);
  BOOL ret = true;
  ret = Process32First (snapshot, &entry);

  while (ret && entry.th32ProcessID != id) {
    ret = Process32Next (snapshot, &entry);
  }

  CloseHandle (snapshot);
  return ret ? entry.cntThreads : 0;
#else
#error Not implemented on this platform
#endif
}
#endif

#ifdef __linux__
static int
getMaxThreads ()
{
  if (maxThreads == 0) {
    struct rlimit limits;
    getrlimit (RLIMIT_NPROC, &limits);

    maxThreads = limits.rlim_cur;
  }

  return maxThreads;
}

static void
checkThreads (float limit_percent)
{
  int nThreads;
  int maxThreads = getMaxThreads ();

  if (maxThreads <= 0) {
    return;
  }

  nThreads = getNumberOfThreads ();

  if (nThreads > maxThreads * limit_percent ) {
    throw KurentoException (NOT_ENOUGH_RESOURCES, "Too many threads");
  }
}

static int
getMaxOpenFiles ()
{
  if (maxOpenFiles == 0) {
    struct rlimit limits;
    getrlimit (RLIMIT_NOFILE, &limits);

    maxOpenFiles = limits.rlim_cur;
  }

  return maxOpenFiles;
}

static int
getNumberOfOpenFiles ()
{
  int openFiles = 0;
  DIR *d;
  struct dirent *dir;

  d = opendir ("/proc/self/fd");

  while ( (dir = readdir (d) ) != NULL) {
    openFiles ++;
  }

  closedir (d);

  return openFiles;
}

static void
checkOpenFiles (float limit_percent)
{
  int nOpenFiles;
  int maxOpenFiles = getMaxOpenFiles ();

  if (maxOpenFiles <= 0) {
    return;
  }

  nOpenFiles = getNumberOfOpenFiles ();

  if (nOpenFiles > maxOpenFiles * limit_percent ) {
    throw KurentoException (NOT_ENOUGH_RESOURCES, "Too many open files");
  }
}
#endif

#ifdef _WIN32
static void
checkMemory (float limit_percent)
{
  MEMORYSTATUSEX statex;

  statex.dwLength = sizeof (statex);

  GlobalMemoryStatusEx (&statex);

  if (statex.dwMemoryLoad > 100 * limit_percent) {
    throw KurentoException (NOT_ENOUGH_RESOURCES, "Insufficient memory");
  }
}
#endif

void
checkResources (float limit_percent)
{
#ifdef __linux__
  checkThreads (limit_percent);
  checkOpenFiles (limit_percent);
#elif defined(_WIN32)
  checkMemory (limit_percent);
#endif
}

void killServerOnLowResources (float limit_percent)
{
  MediaSet::getMediaSet()->signalEmptyLocked.connect ([limit_percent] () {
    GST_DEBUG ("MediaSet empty, checking resources");

    try {
      checkResources (limit_percent);
    } catch (KurentoException &e) {
      if (e.getCode() == NOT_ENOUGH_RESOURCES) {
        GST_ERROR ("Resources over the limit, server will be killed");
#ifdef __linux__
        kill ( getpid(), SIGTERM );
#else
        exit (0);
#endif
      }
    }
  });
}

} /* kurento */

static void init_debug (void) __attribute__ ( (constructor) );

static void
init_debug (void)
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}
