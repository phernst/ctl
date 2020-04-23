# Changelog

## 2020-04-23 (v.0.3.2)

FOCUS: GUI improvements

### Added core functionality
- added `Chunk2DView`: widget for visualization of `Chunk2D` data, also used for further widgets
- added new submodule 'gui_widgets_charts.pri' containing widgets for several "plot-like" visualizations:
  - `LineSeriesView`: visualizer for `XYDataSeries` data - a classic "x-y-style" plot
  - `IntervalSeriesView`: visualizer for `IntervalSeries` data - a "bar-style" plot
  - `DataModelViewer`: complex viewer for inspection of data models (any `AbstractDataModel` subclass) ; provides several visualization and manipulation options.
- included several interaction options with visualizers (e.g. windowing with mouse-gestures, plotting of contrast lines, saving screenshots); refer to the documentation of each class for full details.
- all visualizations can be invoked using a common function: `gui::plot()` with the input argument that shall be visualized, for maximal user convenience and slim code

### Changed
- moved all GUI classes to namespace `CTL::gui`
- fully reworked widget for visualization of projection data: `ProjectionViewer` (replaces previous ProjectionView)
- fully reworked widget for visualization of volume data: `VolumeViewer` (replaces previous VoxelVolumeView); now allows visualization of different volume types (i.e. `VoxelVolume<T>`, `SpectralVolumeData`, `CompositeVolume`)
- refactored widget for computation and visualization of arbitrary slices through volume data: `VolumeSliceViewer` (replaces previous VolumeSlicerWidget)
- renamed PipelineComposerWidget to `PipelineComposer`, added `DynamicProjectorExtension` as a possible extension
- renamed AcquisitionVisualizerWidget to `AcquisitionSetupView`
- renamed SystemVisualizerWidget to `CTSystemView`


Other changes and additions:
- changed spelling of `CTsystem` to `CTSystem` in all its occurrances (i.e. also in member methods and other classes such as `SimpleCTsystem`); old spelling will be kept as deprecated for a while, please consider renaming
- added `AbstractDynamicVolumeData::timeCurve()` method to extract values for all time points of a single voxel ("time-attenuation-curve")
- added `BasisFunctionVolume`: describes time dynamic by a linear combination of time-dependent basis functions 
- added convenience function `makeCTsystem()` and `makeSimpleCTsystem()` for creating systems from blueprints; add `AcquisitionSetup` constructors accepting `unique_ptr`
- implemented parallelization of arithmetic operations with `VoxelVolume<T>`
- added meta headers for each main module - you can now include a single meta header (e.g. ctl.h) to include all headers from the corresponding ctl module.

Note for developers who have already been using the dev_v0.3.2 branch: the namespace `CTL::gui::assist`, which previously contained the plot functions, has been dropped for the sake of simplicity. The all `plot()` overloads can now be found in `CTL::gui` namespace.

## 2020-03-20 (v.0.3.1)

**Important change for existing users:**

`AbstractProjector::configure()` now only takes one parameter ('AcquisitionSetup'). All previous code must be adjusted accordingly - sorry for that!
Settings for the actual projector are now done using corresponding setter methods of the projector class itself.


### Added core functionality

FOCUS: Projector and Extension refactoring

Major convenience features:
- added a substantial amount of documentation and example code to many classes
- added two new convenience classes: `ProjectionPipeline` and `StandardPipeline`

    `ProjectionPipeline` provides a simple means to manage a projector along with an arbitrary number of `ProjectorExtension` objects. It allows for manipulations of the processing pipeline in a list-like fashion.
    `StandardPipeline` goes even one step further and provides a pre-defined pipeline that is easily configurable using some well-defined setter methods.

- added a fully-functional GUI widget (`PipelineComposerWidget`) to compose a `ProjectionPipeline` incl. all configuration options. This class also provides a simple dialog option (`PipelineComposerWidget::fromDialog()`), allowing you to create your projection pipeline on demand.

Overall, putting together your desired simulation settings should now be considerably simpler than before!

### Changed
Changes to projectors and extensions:
- corrected several errors
- all projectors and extensions are now serializable
- all extensions now provide constructors with direct member initialization
- removed `AbstractProjectorConfig` parameter from `AbstractProjector::configure()` - Specific settings for projectors must now be set by means of dedicated setter methods, similar to how it was done already with extensions.
- `ProjectorNotifier` now supports sending signals with general information as `QString`; included several such information messages in current extensions
- `SpectralEffectsExtension`
    - fully refactored code
    - enabled full compatibility for volume data without spectral information (this holds true also for `CompositeVolume` with mixed subvolumes, i.e. some containing spectral information and some not)
    - delegated extraction of spectral information to `RadiationEncoder`
- `PoissonNoiseExtension`
    - add Gaussian approximation for large photon counts - This change increases computation speed for large counts and avoids some numerical issues with the Poisson distribution of extemely large mean.
- `ArealFocalSpotExtension`
    - now correctly defined to be non-linear
    - added option to perform linear approximation (useful for projections with small extinction gradients)
- `DynamicProjectorExtension` (formerly DynamicProjector)
    - refactored: now of base class `ProjectorExtension`
    - can now properly handle different types of volume data and nested projectors
    - supports CompositeVolumes with mixed subvolumes (i.e. some containing temporal dynamics and some static volumes)

Other changes:
- refactoring of volume data classes:
   - volume classes are now polymorphic
   - added more factory methods to create volume objects with basic geometrical shapes
   - refactored `CompositeVolume`: now much more flexible to use
   - refactored `AbstractDynamicVolumeData` (formerly AbstractDynamicVoxelVolume): now of base class `SpectralVolumeData`
   - added `LinearDynamicVolume`: simple class allowing for linear relation of temporal dependency of attenuation in each voxel
- refactored `TASMIPSpectrumModel`: now more performant and memory-saving
- `RadiationEncoder` can now extract spectral information for an entire setup
- new submodule (gui_widgets_ocl.pri) containing widgets with OpenCL dependency
- added Gaussian filter methods
- add method to directly query the mean energy of a source's spectrum
- add option to connect messages from (Qt) signals to the `MessageHandler`

## 2020-02-03 (v.0.3)
### Added core functionality
FOCUS: Image processing
- Radon transformation of images and volumes on GPU (RadonTransform2D, RadonTransform3D) [reference paper: SPIE Medical Imaging 2020 - ID 11313-87; available soon]
- methods for differentiation and filtering of images and volumes (e.g. Gaussian blur, median, Ram-Lak, central difference, Savitzky-Golay, spectral derivatives)
- fast resampling of images and volume on GPU (ImageResampler, VolumeResampler)
- Grangeat consistency measures:
    - intermediate function computation
    - intermediate function pair generation (projection-volume, projection-projection)
- error metrics: compute different error measures between two 1D datasets (L1, L2, RMSE, correlation, cosine similarity, Geman-McClure)
- VolumeSlicer: allows slicing of voxelized volumes along arbitrary planes
- VolumeDecomposer: decompose voxelized volumes of attenuation coefficients into (multiple) volumes of density for different materials
    - generic interface + specific implementation using data models (ModelBasedVolumeDecomposer)
    - simple threshold based segmentation for two materials (TwoMaterialThresholdVolumeDecomposer)

Polychromatic simulations
- AttenuationFilter class: beam modifier applying monomaterial Lambert-Beer attenuation (incl. full consideration in projections)
- RadiationEncoder class: manages information on radiation - ie. flux and spectrum - of CT system as a whole (analogue to GeometryEncoder for system geometry)
- TASMIP X-ray spectrum model (realistic X-ray spectra for tungsten tubes up to 140 keV)
- spectral response for detector (incl. consideration in projector extension)
- add option to (artificially) restrict energy range of source components (main use in spectral projections)

Miscellaneous
- MessageHandler: fully customize messages (control verbosity, appearance etc.) in CTL; can log to file
- more data models: step functions (e.g. constant, rect)
- operators on data models (arithmetics, concatenation etc.)
- database now includes material densities
- homography class (matrix subclass)
- range & coordinate classes

### Changed
- new structure for CTL modules: strictly dependency-based main modules + semantically-grouped sub modules
- refactored AbstractBeamModifier class: new interface with separate modification of flux and spectrum
- refactored XrayTube class: now always uses a TASMIP model for spectrum and flux estimation
- refactored SpectralVolumeData: can now retain spectral information when managing mu values instead of material densities
- renamed SpectralProjectorExtension to SpectralEffectsExtension, as it now considers several spectral effects

### Widgets and Apps 
- VolumeSlicerWidget: GUI widget to conveniently make use of new VolumeSlicer class (incl. visualization of the slice location)
- ProjectorAssemblyWidget: provides assessment of (physical) meaningfulness/exactness of projector extension sequences; can generate corresponding source code
- [regist2d3d.pro] 2D/3D registration app: full toolchain to register geometry of a 2D projection image to a given volume intermediate function (Grangeat representation of a voxelized volume) [reference paper: currently under review]
- [radon3d.pro] 3D intermediate function (pre-)computation app: transforms a voxelized volume to its (Grangeat) intermediate space

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
