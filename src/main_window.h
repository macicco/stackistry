/*
Stackistry - astronomical image stacking
Copyright (C) 2016 Filip Szczerek <ga.software@yahoo.com>

This file is part of Stackistry.

Stackistry is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Stackistry is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Stackistry.  If not, see <http://www.gnu.org/licenses/>.

File description:
    Main window header.
*/

#ifndef STACKISTRY_MAIN_WINDOW_HEADER
#define STACKISTRY_MAIN_WINDOW_HEADER

#include <climits>
#include <cstddef>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include <cairomm/context.h>
#include <gdkmm/pixbuf.h>
#include <glibmm/threads.h>
#include <glibmm/dispatcher.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/toggleaction.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/window.h>
#include <gtkmm/statusbar.h>
#include <skry/skry_cpp.hpp>

#include "img_viewer.h"


const size_t NONE = SIZE_MAX;

class c_MainWindow: public Gtk::Window
{
public:
    c_MainWindow();
    virtual ~c_MainWindow();

    void Finalize();

private:
    c_ImageViewer m_Visualization;
    Gtk::Paned m_MainPaned;
    Glib::RefPtr<Gtk::ActionGroup> m_ActionGroup;
    Glib::RefPtr<Gdk::Pixbuf> m_VisualizationImg;
    Glib::RefPtr<Gtk::UIManager> m_UIManager;
    Gtk::Statusbar m_StatusBar;
    Glib::RefPtr<Gtk::ToggleAction> m_ActVisualization;

    struct Job_t
    {
        libskry::c_ImageSequence imgSeq; // has to be the first field
        float refPtPlacementThreshold; ///< Relative brightness threshold for automatic placement
        libskry::c_Image stackedImg;
        Utils::Const::OutputSaveMode outputSaveMode;
        enum SKRY_quality_criterion qualityCriterion;
        unsigned qualityThreshold; ///< Interpreted according to 'qualityCriterion'
        enum SKRY_output_format outputFmt;
        std::string sourcePath; ///< For image series: directory only; for videos: full path to the video file
        std::string destDir; // effective if outputSaveMode==OutputSaveMode::SPECIFIED_PATH
        std::vector<struct SKRY_point> anchors; // if empty, video stabilization anchors will be set automatically
        bool automaticRefPointsPlacement;
        std::vector<struct SKRY_point> refPoints; // used when 'automaticRefPointsPlacement' is false
        std::string flatFieldFileName; // if empty, no flat-fielding will be performed
        unsigned refPtSpacing;
        /// If not SKRY_CFA_NONE, mono images will be treated as raw color with this filter pattern
        enum SKRY_CFA_pattern cfaPattern;
    };

    class c_JobsListModelColumns: public Gtk::TreeModelColumnRecord
    {
    public:
        Gtk::TreeModelColumn<Glib::ustring> jobSource;
        Gtk::TreeModelColumn<Glib::ustring> state;
        Gtk::TreeModelColumn<size_t> progress;
        Gtk::TreeModelColumn<unsigned> percentageProgress;
        Gtk::TreeModelColumn<Glib::ustring> progressText;
        // Thanks to 'shared_ptr' we can emplace a new 'Job_t' in 'm_Jobs.data' without
        // the need to copy anything (as libskry::c_ImageSequence is non-copyable).
        // Cannot use 'unique_ptr', because a model column itself needs to be copyable.
        Gtk::TreeModelColumn<std::shared_ptr<Job_t>> job;

        c_JobsListModelColumns()
        {
            add(jobSource);
            add(state);
            add(progress);
            add(percentageProgress);
            add(progressText);
            add(job);
        }
    };

    struct
    {
        c_JobsListModelColumns       columns;
        Glib::RefPtr<Gtk::ListStore> data;
        Gtk::TreeView                view;
    } m_Jobs;

    /// Last step for which a notification has been received from worker; may equal NONE
    size_t m_LastStepNotify;

    Gtk::ListStore::iterator m_RunningJob; ///< May be null

    std::queue<Gtk::TreeModel::Path> m_JobsToProcess;


    // Signal handlers -------------
    void OnButtonClicked();
    void OnJobCursorChanged();
    bool OnJobBtnPressed(GdkEventButton *event);
    void OnAddFolders();
    void OnAddImageSeries();
    void OnWorkerProgress();
    void OnStartProcessing();
    void OnStopProcessing();
    void OnPauseResumeProcessing();
    void OnQualityTest();
    void OnSetAnchors();
    void OnSaveStackedImage();
    void OnSelectFrames();
    bool OnDelete(GdkEventAny *event);
    void OnPreferences();
    void OnAddVideos();
    void OnSettings();
    void OnToggleVisualization();
    void OnBackgroundTest();
    void OnSelectionChanged();
    void OnRemoveJobs();
    void OnCreateFlatField();
    void OnQuit();
    void OnAbout();
    //------------------------------

    Job_t &GetJobAt(const Gtk::TreeModel::Path &path);
    Job_t &GetJobAt(const Gtk::ListStore::iterator &iter);
    void InitActions();
    void InitControls();
    void CreateJobsListView();
    void PrepareDialog(Gtk::Dialog &dlg);
    void ClearRunningJobRef();
    void SetStatusBarText(const Glib::ustring &text);
    Gtk::TreeModel::Path GetJobsListFocusedRow();
    Job_t &GetCurrentJob();
    void SetToolbarIcons();
    Gtk::ToolButton *GetToolButton(const char *actionName);
    void SetDefaultSettings(c_MainWindow::Job_t &job);
    void AutoSaveStack(const Job_t &job);
    bool SetAnchorsAutomatically(Job_t &job); ///< Returns false on failure
    /** Sets the enabled state of certain actions depending
        on current processing state and jobs list selection. */
    void UpdateActionsState();
    /// Returns 'false' if user canceled the selection
    bool SetAnchors(Job_t &job);
};

#endif // STACKISTRY_MAIN_WINDOW_HEADER
