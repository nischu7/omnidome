#include <omni/ui/proj/Tuning.h>

#include <vector>

#include <QDebug>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QDrag>
#include <QMimeData>

#include <omni/ui/proj/TitleBar.h>
#include <omni/ui/TuningGLView.h>
#include <omni/proj/FreeSetup.h>
#include <omni/proj/PeripheralSetup.h>

namespace omni
{
  namespace ui
  {
    namespace proj
    {
      Tuning::Tuning(
        int _index,
        omni::Session* _session,
        QWidget* _parent) :
        ParameterWidget(_parent)
      {
        setup();
        setTuning(_index,_session);
      }

      Tuning::Tuning(
        QWidget* _parent) :
        ParameterWidget(_parent)
      {
        setup();
      }

      Tuning::~Tuning()
      {
      }

      omni::proj::Tuning* Tuning::tuning()
      {
        return session_ ? session_->tunings()[index_] : nullptr;
      }

      omni::proj::Tuning const* Tuning::tuning() const
      {
        return session_ ? session_->tunings()[index_] : nullptr;
      }

      void Tuning::setTuning(int _index, omni::Session* _session)
      {
        bool _newTuning = (session_ != _session) || (index_ != _index);
        index_=_index;
        session_ = _session;
   
        if (_newTuning) 
        {
          glView_->setSession(session_);
          glView_->setTuningIndex(index_);
          glView_->setBorder(0.0);
          glView_->setKeepAspectRatio(false);
          glView_->setViewOnly(true);
          glView_->update();
          titleBar_->setColor(tuning()->color());

          // Also attach fullscreen
          if (fullscreen_)
          {
            fullscreen_->setSession(session_);
            fullscreen_->setTuningIndex(index_);
          }
          sessionModeChange();
        }
      }
        
      /// Enable or disable fullscreen display
      void Tuning::fullscreenToggle(bool _enabled)
      {
        glView_->setDrawingEnabled(_enabled);
        if (fullscreen_)
        {
          fullscreen_->setDrawingEnabled(_enabled);
        }
      }
 
      int Tuning::index() const
      {
        return index_;
      }
        
      Session const* Tuning::session() const
      {
        return session_;
      }
 
      void Tuning::updateParameters()
      {
        if (!tuning()) return;

        auto* _projSetup = tuning()->projectorSetup();
        
        if (!_projSetup) return;
 
        /// Handle free projector setup
        if (_projSetup->getTypeId() == "FreeSetup")
        {
          auto* _p = static_cast<omni::proj::FreeSetup*>(_projSetup);
          _p->setYaw(getParamAsFloat("Yaw"));
          _p->setPitch(getParamAsFloat("Pitch"));
          _p->setRoll(getParamAsFloat("Roll"));
          _p->setPos(
              getParamAsFloat("X"),
              getParamAsFloat("Y"),
              getParamAsFloat("Z"));
        } 

        /// Handle Peripheral projector setup 
        if (_projSetup->getTypeId() == "PeripheralSetup")
        {
          auto* _p = static_cast<omni::proj::PeripheralSetup*>(_projSetup);
         
          _p->setYaw(getParamAsFloat("Yaw"));
          _p->setPitch(getParamAsFloat("Pitch"));
          _p->setRoll(getParamAsFloat("Roll"));
          _p->setDeltaYaw(getParamAsFloat("Delta Yaw"));
          _p->setDistanceCenter(getParamAsFloat("Distance"));
          _p->setTowerHeight(getParamAsFloat("Tower Height"));
          _p->setShift(getParamAsFloat("Shift"));

        }
        tuning()->setupProjector();

        updateViews();
        emit projectorSetupChanged();
      }

        
      void Tuning::attachScreen(Screen const& _screen)
      {
        fullscreen_.reset(new TuningGLView());

        fullscreen_->hide();
        fullscreen_->setSession(session_);
        fullscreen_->setTuningIndex(index_);
        fullscreen_->setFullscreen(_screen);
      }

      void Tuning::detachScreen()
      {
        fullscreen_.reset();
      }

      void Tuning::updateViews()
      {
        glView_->update();
        if (fullscreen_) 
          fullscreen_->update();
      }

      void Tuning::setup()
      {
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

        /// Setup title bar
        titleBar_ = new TitleBar("Projector",this);
        titleBar_->installEventFilter(this);
        connect(titleBar_,SIGNAL(closeButtonClicked()),this,SLOT(prepareRemove()));

        /// Setup preview window
        glView_ = new TuningGLView(this);
        QSizePolicy _sizePolicy(QSizePolicy::Ignored,QSizePolicy::Expanding);
        glView_->setSizePolicy(_sizePolicy);
        glView_->setKeepAspectRatio(true);
        glView_->installEventFilter(this);

        /// FOV view slider
        /// @todo Connect this with threshold slider
        auto* _fov =  addWidget("FOV",60.0,10.0,160.0);
        _fov->setSingleStep(4.0);
        _fov->setPageStep(45.0);
        _fov->setSuffix("°");

        /// Throw ratio slider
        /// @todo Connect this with FOV slider
        auto* _throwRatio = addWidget("Throw Ratio",1.0,0.1,5.0);
        _throwRatio->setSingleStep(0.1);
        _throwRatio->setPageStep(0.3);

        /// Yaw angle slider (all projector setups)
        auto&& _yaw = addAngleWidget("Yaw",0.0,0.0,360.0);

        /// Tower height slider (PeripheralSetup only)
        auto&& _towerHeight = addOffsetWidget("Tower Height",2.0,-5.0,10.0);
        _towerHeight->setSingleStep(0.1);
        _towerHeight->setPageStep(1.0);
        _towerHeight->setPivot(0.0);

        /// Distance slider (PeripheralSetup only)
        auto&& _distance = addOffsetWidget("Distance",5.0,0.0,10.0);
        _distance->setPageStep(1.0);

        /// Shift offset slider (PeripheralSetup only)
        auto&& _shift = addOffsetWidget("Shift",0.0,-2.0,2.0);
        _shift->setPageStep(1.0);
        _shift->setPivot(0.0);
        
        /// X offset slider (FreeSetup only)
        auto&& _x = addOffsetWidget("X",0.0,-10.0,10.0);
        _x->setPageStep(1.0);
        _x->setPivot(0.0);
        
        /// Y offset slider (FreeSetup only)
        auto&& _y = addOffsetWidget("Y",0.0,-10.0,10.0);
        _y->setPageStep(1.0);
        _y->setPivot(0.0);
        
        /// Z offset slider (FreeSetup only)
        auto&& _z = addOffsetWidget("Z",0.0,-10.0,10.0);
        _z->setPageStep(1.0);
        _z->setPivot(0.0);

        /// Pitch angle slider (both setups)
        auto&& _pitch = addAngleWidget("Pitch",30.0,-90.0,90.0);
        _pitch->setPivot(0.0);

        /// Roll angle slider (both setups)
        auto&& _roll = addAngleWidget("Roll",0.0,-45.0,45.0);
        _roll->setSingleStep(1.0);
        _roll->setPageStep(5.0);
        _roll->setPivot(0.0);

        /// Delta yaw angle slider (PeripheralSetup only)
        auto&& _deltaYaw = addAngleWidget("Delta Yaw",0.0,-45.0,45.0);
        _deltaYaw->setSingleStep(1.0);
        _deltaYaw->setPageStep(5.0);
        _deltaYaw->setPivot(0.0);
        
        /// Make slider groups 
        sliderGroups_["FreeSetup"] = {_yaw,_roll,_pitch,_x,_y,_z};
        sliderGroups_["PeripheralSetup"] = {_yaw,_distance,_shift,_towerHeight,_pitch,
          _deltaYaw,_roll};
        sliderGroups_["FOV"] = { _fov, _throwRatio };
        
        /// Setup/update mode
        sessionModeChange();
      }

      void Tuning::reorderWidgets()
      {
        if (!titleBar_ || !tuning()) return;

        const int _border = 2;
        int _height = _border;

        /// Hide all widgets
        glView_->hide();
        for (auto& _slider : this->parameters_)
          _slider->hide();


        /// Our widget list
        std::vector<QWidget*> _widgets = 
        {
          titleBar_
        };

        /// Add preview widget 
        if (windowState_ != NO_DISPLAY)
        {
            _widgets.push_back(glView_); 
        }

        /// Add adjustment sliders
        if (windowState_ == ADJUSTMENT_SLIDERS)
        {
          for (auto& _slider : sliderGroups_.at(tuning()->projectorSetup()->getTypeId().str()))
          {
            _widgets.push_back(_slider);
          } 
        }

        /// Add FOV sliders
        if (windowState_ == FOV_SLIDERS)
        {
              for (auto& _slider : sliderGroups_.at("FOV"))
              {
                _widgets.push_back(_slider);
              } 
        }

        /// Adjust geometry for each widget
        for (auto& _widget : _widgets)
        {
          /// Widget height is constant except for preview
          int _widgetHeight = _widget == glView_ ? width() / 4.0 * 3.0 : 25;
          _widget->setParent(this);
          _widget->setGeometry(_border,_height,width()-_border*2,_widgetHeight);
          _widget->show();
          
          /// Increase height
          _height += _widgetHeight;
        }
        _height += _border;
        
        /// Set minimum size and resize 
        setMinimumSize(0,_height);
        resize(width(),_height);
      }
  
      Tuning::WindowState Tuning::windowState() const
      {
        return windowState_;
      }

      void Tuning::setWindowState(WindowState _windowState)
      {
        windowState_ = _windowState;
        reorderWidgets();
      }

      void Tuning::setNextWindowState()
      {
        setWindowState(static_cast<WindowState>((int(windowState_) + 1) % int(NUM_WINDOW_STATES)));
      }

      void Tuning::setSelected(bool _isSelected)
      {
        isSelected_ = _isSelected;
        if (isSelected_) emit selected();

        updateColor();
      }

      void Tuning::updateColor()
      {
        /// Widget color has the same color as tuning
        for (auto& _widget : this->parameters_)
        { 
          QColor _color = isSelected_ ? titleBar_->color().name() : "#cccccc";
          _widget->setStyleSheet("selection-background-color  : "+_color.name());
 
          if (tuning() && isSelected_)
            tuning()->setColor(_color);
        }
        update();
      }
        
      void Tuning::prepareRemove()
      {
        emit closed(index_);
      }

      void Tuning::sessionModeChange() 
      {
        if (!session()) return;

        switch (session()->mode())
        {
          case Session::Mode::SCREENSETUP:
          case Session::Mode::PROJECTIONSETUP:
          case Session::Mode::WARP:
          case Session::Mode::BLEND:
          case Session::Mode::EXPORT:
            break;
          default: break;
        }

        reorderWidgets();
      }

      void Tuning::resizeEvent(QResizeEvent* event)
      {
        QWidget::resizeEvent(event);
        update();
      }
   
      void Tuning::showEvent(QShowEvent*)
      {
        reorderWidgets();
      }

      void Tuning::paintEvent(QPaintEvent* event)
      {
        if (!titleBar_) return;

        QPainter _p(this);

        auto _rect = rect().adjusted(2,2,-2,-2);
        
        /// Paint board if active or color is selected
        if (isSelected_)
        {
          _p.setPen(QPen(titleBar_->color(),5));
        } else
        {
          _p.setPen(Qt::NoPen);
        }
        
        _p.setBrush(titleBar_->color());
        _p.drawRect(_rect);

        QWidget::paintEvent(event);
      }
        
      /// Mouse Move Event and handler for dragging to ScreenSetup widget
      void Tuning::mouseMoveEvent(QMouseEvent* event)
      {   
        // Handle drag to ScreenWidget
        if (event->button() == Qt::LeftButton) 
        {
          startDrag();
        }
      }
 
      void Tuning::startDrag()
      {
        qDebug() << "startDrag ";
          QDrag *drag = new QDrag(this);
          QMimeData *mimeData = new QMimeData;

          mimeData->setText("Hallo"); // TODO add tuning index here
          drag->setMimeData(mimeData);
      }

      bool Tuning::eventFilter(QObject* _obj, QEvent* _event)
      {
        if (_event->type() == QEvent::MouseMove && (_obj == glView_ || _obj == titleBar_)) 
        {
          startDrag();
        }

        /// Handle focus events
        if (_event->type() == QEvent::FocusIn)
        {
          setSelected(true);
          return true;
        } else
        if (_event->type() == QEvent::FocusOut)
        {
          setSelected(false);
          return false;
        }
          
        return false;
      }
        
      bool Tuning::isSelected() const
      {
        return isSelected_;
      }
 
      /// Focus event used by TuningList to set current tuning for session
      void Tuning::focusInEvent(QFocusEvent* event)
      {
        setSelected(true);
        QWidget::focusInEvent(event);
      }

      void Tuning::focusOutEvent(QFocusEvent* event)
      {
        setSelected(false);
        QWidget::focusOutEvent(event);
      }
    }
  }
}
