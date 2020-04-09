
add_library(Qt5::QGeoPositionInfoSourceFactoryGeoclue MODULE IMPORTED)

_populate_Positioning_plugin_properties(QGeoPositionInfoSourceFactoryGeoclue RELEASE "position/qtposition_geoclue.dll")
_populate_Positioning_plugin_properties(QGeoPositionInfoSourceFactoryGeoclue DEBUG "position/qtposition_geoclued.dll")

list(APPEND Qt5Positioning_PLUGINS Qt5::QGeoPositionInfoSourceFactoryGeoclue)
