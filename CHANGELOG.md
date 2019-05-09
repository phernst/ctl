# Changelog

## 2019-05-07 (v.0.2)
### Added
- major
    - SpectralProjectorExtension: allows for computation of polychromatic projections
    - NRRD file support (read and write)
    - physical volume objects 
        - SpectralVolumeData: combines density data with a model for spectrally-dependent absorption coefficient
        - CompositeVolume: container for multiple SpectralVolumeData objects
    - projectComposite() option in projectors: can compute projections from composite volumes
    - database with many tabulated absorption spectra and an exemplary X-ray spectrum
- minor
    - global gantry displacement
    - GenericDetector class now supports changing pixel size and skew coefficient
    - dedicated classes for SingleViewGeoemetry and FullGeometry
    - HeuristicCubicSpectrumModel: simple model for approximate representation of Xray tube spectra
    - fixed seed option in PoissonNoiseExtension
    - sources now provide the energy range of their emitted radiation

### Changed
- redesigned spectrum queries from source components
- new design for ProjectorExtension
- reworked calculation of photon flux in source components: now uses realistic properties of tube and laser

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
