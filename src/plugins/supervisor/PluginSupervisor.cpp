/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2013, Institute for Artificial Intelligence,
 *  Universität Bremen.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Institute for Artificial Intelligence,
 *     Universität Bremen, nor the names of its contributors may be
 *     used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/** \author Jan Winkler */


#include <plugins/supervisor/PluginSupervisor.h>


namespace beliefstate {
  namespace plugins {
    PLUGIN_CLASS::PLUGIN_CLASS() {
      m_bFirstExperiment = true;
      this->setPluginVersion("0.6");
    }
    
    PLUGIN_CLASS::~PLUGIN_CLASS() {
    }
    
    Result PLUGIN_CLASS::init(int argc, char** argv) {
      Result resInit = defaultResult();
      
      this->setSubscribedToEvent("startup-complete", true);
      this->setSubscribedToEvent("start-new-experiment", true);
      this->setSubscribedToEvent("shutdown", true);
      
      return resInit;
    }
    
    Result PLUGIN_CLASS::deinit() {
      // When shutting down, check for .owl files in the experiment
      // directory. If there are none, delete the entire experiment
      // directory. NOTE(winkler): This might get extended in order
      // to support file types in the configuration file. In the
      // current state, situations that would only generate .dot
      // files (even if this is the desired output) would result in
      // a deleted directory. Shouldn't come up at the moment,
      // though.
      ConfigSettings cfgsetCurrent = configSettings();
      
      if(!this->extensionPresent(cfgsetCurrent.strExperimentDirectory, "owl")) {
	// No .owl files found in experiment directory.
	this->info("Cleaning up directory, as no valid experiment data was found.");
	
	deleteDirectory(cfgsetCurrent.strExperimentDirectory);
	
	string strSymlinkName = cfgsetCurrent.strBaseDataDirectory + "/" + cfgsetCurrent.strSymlinkName;
	::remove(strSymlinkName.c_str());
      }
      
      return defaultResult();
    }
    
    Result PLUGIN_CLASS::cycle() {
      Result resCycle = defaultResult();
      this->deployCycleData(resCycle);
      
      return resCycle;
    }
    
    bool PLUGIN_CLASS::extensionPresent(string strPath, string strExtension) {
      bool bFound = false;
      DIR* dirFile = opendir(strPath.c_str());
      string strPointExt = "." + strExtension;
      
      if(dirFile) {
	struct dirent* hFile;
	errno = 0;
	while((hFile = readdir(dirFile)) != NULL) {
	  if(!strcmp(hFile->d_name, ".")) continue;
	  if(!strcmp(hFile->d_name, "..")) continue;
	  
	  if(strstr(hFile->d_name, strPointExt.c_str())) {
	    struct stat sb;
	    string strFilepath = strPath + "/" + hFile->d_name;
	    lstat(strFilepath.c_str(), &sb);
	    
	    if((sb.st_mode & S_IFMT) != S_IFLNK) { // Symlinks don't count.
	      bFound = true;
	      break;
	    }
	  }
	}
	
	closedir(dirFile);
      }
      
      return bFound;
    }
    
    void PLUGIN_CLASS::consumeEvent(Event evEvent) {
      if(evEvent.strEventName == "start-new-experiment") {
	bool bWait = true;
	
	// Signal global experiment shut down.
	if(m_bFirstExperiment) {
	  // Only signal an experiment shutdown if this is not the
	  // first experiment.
	  m_bFirstExperiment = false;
	  bWait = false;
	} else {
	  this->deployEvent(defaultEvent("experiment-shutdown"));
	}
	
	if(bWait) {
	  // Wait 1 sec for all plugins to realize what to do when
	  // shutting down an experiment.
	  sleep(1);
	}
	
	ConfigSettings cfgsetCurrent = configSettings();
	// Create base data directory
	mkdir(cfgsetCurrent.strBaseDataDirectory.c_str(), 0777);
	
	// This is a hardcoded length. If the length of the
	// numeric/date identifier in the string actually exceeds
	// this, this will probably result in a segfault. This should
	// very rarely be the case, though. We're assuming no problem
	// here.
	char cName[cfgsetCurrent.strExperimentNameMask.size() + 80];
	
	string strNewName;
	string strNewExp;
	
	if(cfgsetCurrent.strExperimentNameMask.find("%d") != string::npos) {
	  int nIndex = 0;
	  bool bExists;
	
	  do {
	    sprintf(cName, (const char*)(cfgsetCurrent.strExperimentNameMask.c_str()), nIndex);
	    strNewExp = cName;
	    nIndex++;
	    
	    struct stat sb;
	    int nReturnStat = stat(strNewName.c_str(), &sb);
	    bExists = (nReturnStat == 0);
	  } while(bExists);
	} else if(cfgsetCurrent.strExperimentNameMask.find("%s") != string::npos) {
	  std::locale::global(std::locale("en_US.utf8"));
	  std::time_t t = std::time(NULL);
	  char cName2[cfgsetCurrent.strExperimentNameMask.size() + 80];
	  strftime(cName2, cfgsetCurrent.strExperimentNameMask.size() + 80, "%Y-%m-%d_%H-%M-%S", std::localtime(&t));
	  sprintf(cName, (const char*)(cfgsetCurrent.strExperimentNameMask.c_str()), cName2);
	  strNewExp = cName;
	}
	
	strNewName = cfgsetCurrent.strBaseDataDirectory + "/" + strNewExp + "/";
	mkdir(strNewName.c_str(), 0777);
	cfgsetCurrent.strExperimentDirectory = strNewName;
	this->info("Created new experiment space: '" + strNewExp + "'.");
	setConfigSettings(cfgsetCurrent);
	
	Event evSetExpNameMeta = defaultEvent("set-experiment-meta-data");
	evSetExpNameMeta.cdDesignator = new CDesignator();
	evSetExpNameMeta.cdDesignator->setType(ACTION);
	evSetExpNameMeta.cdDesignator->setValue("field", "experiment-name");
	evSetExpNameMeta.cdDesignator->setValue("value", strNewExp);
	this->deployEvent(evSetExpNameMeta);
	
	string strSymlink = cfgsetCurrent.strBaseDataDirectory + "/" + cfgsetCurrent.strSymlinkName;
	remove(strSymlink.c_str());
	symlink(strNewName.c_str(), strSymlink.c_str());
	this->info("Symlink set accordingly.");
	
	CDesignator* cdConfig = this->getIndividualConfig();
	string strKnowRobOwl = cdConfig->stringValue("knowrob-symlink-path");
	
	if(strKnowRobOwl != "") {
	  symlink(strKnowRobOwl.c_str(), string(strNewName + "knowrob.owl").c_str());
	  this->info("Created KnowRob OWL symlink to: '" + strKnowRobOwl + "'");
	}
	
	// Signal global experiment start.
	this->deployEvent(defaultEvent("experiment-start"));
      } else if(evEvent.strEventName == "startup-complete") {
	this->deployEvent(defaultEvent("start-new-experiment"));
      } else if(evEvent.strEventName == "shutdown") {
	this->deployEvent(defaultEvent("experiment-shutdown"));
      }
    }
  }
  
  extern "C" plugins::PLUGIN_CLASS* createInstance() {
    return new plugins::PLUGIN_CLASS();
  }
  
  extern "C" void destroyInstance(plugins::PLUGIN_CLASS* icDestroy) {
    delete icDestroy;
  }
}
