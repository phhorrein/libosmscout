#ifndef MAPWIDGET_H
#define MAPWIDGET_H

/*
  OSMScout - a Qt backend for libosmscout and libosmscout-map
  Copyright (C) 2010  Tim Teulings
  Copyright (C) 2016  Lukáš Karas

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include <QQuickPaintedItem>

#include <osmscout/GeoCoord.h>
#include <osmscout/util/GeoBox.h>

#include <osmscout/private/ClientQtImportExport.h>

#include <osmscout/DBThread.h>
#include <osmscout/SearchLocationModel.h>
#include <osmscout/RoutingModel.h>
#include <osmscout/InputHandler.h>
#include <osmscout/Way.h>

/**
 * \defgroup QtAPI Qt API
 * 
 * Classes for integration osmscout library with Qt framework.
 */

/**
 * \ingroup QtAPI
 * 
 * Qt Quick widget for displaying map. 
 * 
 * Type should to be registered by \ref qmlRegisterType method 
 * and \ref DBThread instance should be initialized before first usage.
 * 
 */
class OSMSCOUT_CLIENT_QT_API MapWidget : public QQuickPaintedItem
{
  Q_OBJECT
  Q_ENUMS(NodesType)
  Q_ENUMS(WaysType)
  Q_PROPERTY(QObject  *view    READ GetView     WRITE SetMapView  NOTIFY viewChanged)
  Q_PROPERTY(double   lat      READ GetLat      NOTIFY viewChanged)
  Q_PROPERTY(double   lon      READ GetLon      NOTIFY viewChanged)
  Q_PROPERTY(int      zoomLevel READ GetMagLevel NOTIFY viewChanged)
  Q_PROPERTY(QString  zoomLevelName READ GetZoomLevelName NOTIFY viewChanged)
  Q_PROPERTY(double   pixelSize READ GetPixelSize NOTIFY viewChanged)
  Q_PROPERTY(bool     databaseLoaded READ isDatabaseLoaded NOTIFY databaseLoaded)
  Q_PROPERTY(bool     finished READ IsFinished  NOTIFY finishedChanged)
  Q_PROPERTY(bool     showCurrentPosition READ getShowCurrentPosition WRITE setShowCurrentPosition)
  Q_PROPERTY(bool     lockToPosition READ isLockedToPosition WRITE setLockToPosition NOTIFY lockToPossitionChanged)
  Q_PROPERTY(QString  stylesheetFilename READ GetStylesheetFilename NOTIFY stylesheetFilenameChanged)
  
  Q_PROPERTY(bool stylesheetHasErrors           READ stylesheetHasErrors              NOTIFY styleErrorsChanged)
  Q_PROPERTY(int stylesheetErrorLine            READ firstStylesheetErrorLine         NOTIFY styleErrorsChanged)
  Q_PROPERTY(int stylesheetErrorColumn          READ firstStylesheetErrorColumn       NOTIFY styleErrorsChanged)
  Q_PROPERTY(QString stylesheetErrorDescription READ firstStylesheetErrorDescription  NOTIFY styleErrorsChanged)  

public:
  enum NodesType {
      WayPointNode,
      EndPointNode,
      PositionNode,
  };

  enum WaysType {
      RouteWay,
  };


private:

  MapView          *view;
  double           mapDpi;

  InputHandler     *inputHandler;
  TapRecognizer    tapRecognizer;     
  
  bool showCurrentPosition;
  bool finished;
  QTime lastUpdate;
  bool locationValid;
  osmscout::GeoCoord currentPosition;
  bool horizontalAccuracyValid;
  double horizontalAccuracy;

  struct NodeDescription {
      osmscout::GeoCoord coords;
      NodesType type;
  };

  struct WayDescription {
      osmscout::WayRef way;
      WaysType type;
  };

  
  QMap<int, osmscout::GeoCoord> marks;
  QMap<int, NodeDescription> nodes;
  QMap<int, WayDescription> ways;
  int currentNodeId;
  int currentWayId;

signals:
  void viewChanged();
  void lockToPossitionChanged();
  void finishedChanged(bool finished);

  void mouseMove(const int screenX, const int screenY, const double lat, const double lon, const Qt::KeyboardModifiers modifiers);
  void tap(const int screenX, const int screenY, const double lat, const double lon);
  void doubleTap(const int screenX, const int screenY, const double lat, const double lon);
  void longTap(const int screenX, const int screenY, const double lat, const double lon);
  void tapLongTap(const int screenX, const int screenY, const double lat, const double lon);

  void stylesheetFilenameChanged();
  void styleErrorsChanged();
  void databaseLoaded(osmscout::GeoBox);
  
public slots:
  void changeView(const MapView &view);
  void redraw();
  
  void recenter();
  
  void zoom(double zoomFactor);
  void zoomIn(double zoomFactor);
  void zoomOut(double zoomFactor);
  
  void zoom(double zoomFactor, const QPoint widgetPosition);
  void zoomIn(double zoomFactor, const QPoint widgetPosition);
  void zoomOut(double zoomFactor, const QPoint widgetPosition);
  
  void move(QVector2D vector);
  void left();
  void right();
  void up();
  void down();
  void rotateLeft();
  void rotateRight();

  void toggleDaylight();
  void reloadStyle();
  void reloadTmpStyle();
  
  void showCoordinates(osmscout::GeoCoord coord, osmscout::Magnification magnification);
  void showCoordinates(double lat, double lon);
  void showCoordinatesInstantly(osmscout::GeoCoord coord, osmscout::Magnification magnification);
  void showCoordinatesInstantly(double lat, double lon);
  void showLocation(LocationEntry* location);

  void locationChanged(bool locationValid, double lat, double lon, bool horizontalAccuracyValid, double horizontalAccuracy);
  
  void addPositionMark(int id, double lat, double lon);
  void removePositionMark(int id);

  int addNode(double lat, double lon, MapWidget::NodesType);
  void removeNode(int id);

  int addWay(osmscout::WayRef way, MapWidget::WaysType);
  int addWay(RoutingListModel *routeModel, MapWidget::WaysType);
  void removeWay(int id);

  bool toggleDebug();
  bool toggleInfo();

private slots:

  void onTap(const QPoint p);
  void onDoubleTap(const QPoint p);
  void onLongTap(const QPoint p);
  void onTapLongTap(const QPoint p);
  
  void onMapDPIChange(double dpi);  
  
private:
  void setupInputHandler(InputHandler *newGesture);
  
  /**
   * @param dimension in kilometers
   * @return approximated magnification by object dimension
   */
  osmscout::Magnification magnificationByDimension(double dimension);
  
public:
  MapWidget(QQuickItem* parent = 0);
  virtual ~MapWidget();

  inline MapView* GetView() const
  {
      return view; // We should be owner, parent is set http://doc.qt.io/qt-5/qqmlengine.html#objectOwnership
  }
  
  inline void SetMapView(QObject *o)
  {
    MapView *updated = dynamic_cast<MapView*>(o);
    if (updated == NULL){
        qWarning() << "Failed to cast " << o << " to MapView*.";
        return;
    }
    
    bool changed = *view != *updated;
    if (changed){
      setupInputHandler(new InputHandler(*updated));
      changeView(*updated);
    }
  }

  inline double GetLat() const
  {
      return view->center.GetLat();
  }

  inline double GetLon() const
  {
      return view->center.GetLon();
  }
  
  inline osmscout::GeoCoord GetCenter() const
  {
      return view->center;
  }

  QString GetStylesheetFilename() const;

  QString GetZoomLevelName() const;

  inline int GetMagLevel() const
  {
      return view->magnification.GetLevel();
  }

  inline double GetPixelSize() const
  {
      return getProjection().GetPixelSize();
  }
  
  inline bool IsFinished() const
  {
      return finished;
  }
  
  inline bool getShowCurrentPosition() const
  { 
      return showCurrentPosition;
  };
  
  inline void setShowCurrentPosition(bool b)
  { 
      showCurrentPosition = b;
  };
  
  inline bool isLockedToPosition()
  {
      return inputHandler->isLockedToPosition();
  };
  
  void setLockToPosition(bool);
  
  inline osmscout::MercatorProjection getProjection() const
  {
    osmscout::MercatorProjection projection;

    size_t w=width();
    size_t h=height();
    projection.Set(GetCenter(),
               view->angle,
               view->magnification,
               mapDpi,
               // to avoid invalid projection when scene is not finished yet
               w==0? 100:w,
               h==0? 100:h);
    return projection;
  }

  void wheelEvent(QWheelEvent* event);
  virtual void touchEvent(QTouchEvent *event);
  
  virtual void focusOutEvent(QFocusEvent *event);

  void translateToTouch(QMouseEvent* event, Qt::TouchPointStates states);
  
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void hoverMoveEvent(QHoverEvent* event);
  
  void paint(QPainter *painter);
  
  bool stylesheetHasErrors() const;
  int firstStylesheetErrorLine() const;
  int firstStylesheetErrorColumn() const;
  QString firstStylesheetErrorDescription() const;
  
  bool isDatabaseLoaded();
  Q_INVOKABLE bool isInDatabaseBoundingBox(double lat, double lon);
};

#endif
