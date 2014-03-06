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


#ifndef __BELIEFSTATE_H__
#define __BELIEFSTATE_H__


// System
#include <iostream>
#include <cstdlib>
#include <list>
#include <string>
#include <libconfig.h++>

// ROS
#include <ros/package.h>

// Private
#include <ForwardDeclarations.h>
#include <Types.h>
#include <PluginSystem.h>
#include <UtilityBase.h>

using namespace std;
using namespace libconfig;


/*! \brief Main beliefstate system class
  
  This class manages the distribution of messages and service events,
  triggers the loading of plugins, and loads configuration files. */
namespace beliefstate {
  class Beliefstate : public UtilityBase {
  private:
    /*! \brief Internal pointer to the PluginSystem class instance
        used for loading and managing plugin instances. */
    PluginSystem* m_psPlugins;
    /*! \brief Flag representing the current run state of the main
        loop cycle. If true, the main loop will continue to run; if
        false, will stop execution of the main loop and quit. */
    bool m_bRun;
    /*! \brief Internal copy of the global argc parameter holding the
        number of command line parameters supplied when starting the
        system. This parameter, together with m_argv, is handed to all
        plugins during initialization. */
    int m_argc;
    /*! \brief Internal copy of the global argv parameter holding the
        command line parameter values supplied when starting the
        system. This parameter, together with m_argc, is handed to all
        plugins during initialization. */
    char** m_argv;
    /*! \brief List of plugins to load. This list is populated by the
        plugins/load configuration section in the configuration
        file. */
    list<string> m_lstPluginsToLoad;
    /*! \brief List of currently available global events to
        process. The entries in this list only last one cycle of the
        main loop. They are dropped afterwards, but are handed to all
        plugins that subscribed to the respective event type
        beforehand. This is the main communication mechanism between
        the belief state system and loaded plugins. */
    list<Event> m_lstGlobalEvents;
    /*! \brief In case a static workspace directory was specified in
        the configuration file, its value is stored here. This value,
        if not empty, takes precedence over the dynamically resolved
        return value of workspaceDirectory(). */
    string m_strWorkspaceDirectory;
    
  public:
    /*! \brief Constructor of the main belief state system class.
      
      \param argc The main argc variable supplied to main(), holding
      the number of available command line parameters issues when
      running the system.

      \param argv The main argv variable supplied to main(), holding
      the actual available command line parameters issues when running
      the system. */
    Beliefstate(int argc, char** argv);
    /*! \brief Destructor of the main belief state system class.
      
      Deletes all instances of internal objects. */
    ~Beliefstate();
    
    Result init(string strConfigFile = "");
    Result deinit();
    
    bool loadConfigFile(string strConfigFile);
    bool loadIndividualPluginConfigurationBranch(Setting &sBranch, CKeyValuePair* ckvpInto, string strConfigPath = "", bool bIgnorePluginField = false);
    void replaceStringInPlace(string& subject, const string& search, const string& replace);
    
    void spreadEvent(Event evEvent);
    void spreadServiceEvent(ServiceEvent seServiceEvent);
    bool cycle();
    
    void triggerShutdown();
    
    void setBaseDataDirectory(string strBaseDataDirectory);
    string baseDataDirectory();
    string resolveDirectoryTokens(string strPath);
    
    string workspaceDirectory();
    string homeDirectory();
  };
}


#endif /* __BELIEFSTATE_H__ */
