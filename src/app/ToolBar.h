/* Copyright (c) 2014-2016 "Omnidome" by cr8tr
 * Dome Mapping Projection Software (http://omnido.me).
 * Omnidome was created by Michael Winkelmann aka Wilston Oreo (@WilstonOreo)
 *
 * This file is part of Omnidome.
 *
 * Omnidome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef OMNI_UI_TOOLBAR_H_
#define OMNI_UI_TOOLBAR_H_

#include <QToolBar>
#include <omni/ui/mixin/SessionWidget.h>

class QToolButton;

namespace omni {
    namespace ui {
        class ToolBar :
            public QToolBar,
            public mixin::SessionWidget
        {
            Q_OBJECT
        public:
            ToolBar(QWidget* = nullptr);
            ~ToolBar();

        public slots:
            void setScreenSetupMode();
            void setArrangeMode();
            void setWarpMode();
            void setBlendMode();
            void setColorCorrectionMode();
            void setExportMode();
            void setLiveMode();
            void setMode(Session::Mode);
            void buttonState();

            /// Show About dialog
            void showSettings();

        signals:
            void sessionModeChanged(Session::Mode);

        private:
            void sessionParameters();

            QToolButton* btnSettings_ = nullptr;
            QToolButton* btnScreenSetup_ = nullptr;
            QToolButton* btnArrange_ = nullptr;
            QToolButton* btnWarp_ = nullptr;
            QToolButton* btnBlend_ = nullptr;
            QToolButton* btnColorCorrection_ = nullptr;
            QToolButton* btnExport_ = nullptr;
            QToolButton* btnLive_ = nullptr;

        };
    }
}

#endif /* OMNI_UI_TOOLBAR_H_ */