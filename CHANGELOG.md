# Changelog

## 2019-02-20
### Added
- complete de-/serialization of AcquisitionSetup and all related classes
- JSON & binary serializer
- detector saturation models
- flying focal spot protocol
- axial scan trajectory
- comparison of projection matrices wrt the projection error: PMatComparator
- abstract IO for writing SingleView & SingleViewGeometry

### Changed
- structure of data models and series

## 2019-01-21
### Added
- RayCasterProjector supports Multi-OpenCL-Device acceleration

### Changed
- RayCasterProjector::Config: has a device list instead of a device ID
