/*  Zynthian core API */

#define PHY_IN 0xFFFFFFFD
#define PHY_OUT 0xFFFFFFFE
#define ALL 0xFFFFFFFF
#define NOT_FOUND 0xFFFFFFFF

/*  Mixer
    There is a stereo summing mixer with one stereo channel strip per chain
    Each channel strip is identified by the associated chain index
    Each channel strip has control of level, balance, mute, solo- & mono (all inputs mixed to both outputs)
    There is a main mix bus with similar control to which all channels are routed
    Instantaneous peak programme and held peak programme for each leg (left/right) of each channel is available
    There is a callback mechanism for state change
*/

/** @brief  Get mixer main mix bus index
*   @retval int Index of channel strip associated with main mix bus
*   @note   This is synonymous with maximum quantity of chains and will change if setMaxChains() is called
*/
int getMainMixbus();

/** @brief  Get mixer channel fader level
*   @param  channel Index of channel
*   @retval float Fader value [0.0..1.0]
*/
float getFaderLevel(int channel);

/** @brief  Set mixer fader level
*   @param  channel Index of channel
*   @param  value Fader value
*/
void setFaderLevel(int channel, float value);

/** @brief  Get mixer channel balance
*   @param  channel Index of channel
*   @retval float Balance value [-1.0..1.0]
*   @note   Balance for stereo source, pan for mono source
*/
float getBalance(int channel);

/** @brief  Set mixer channel balance
*   @param  channel Index of channel
*   @param  value Balance value [-1.0..1.0]
*/
void setBalance(int channel, float value);

/** @brief  Get mixer channel mute state
*   @param  channel Index of channel
*   @retval bool True if channel muted
*/
bool getMute(int channel);

/** @brief  Set mixer channel mute state
*   @param  channel Index of channel
*   @param  state True to mute channel
*/
void setMute(int channel, bool state);

/** @brief  Get mixer channel solo state
*   @param  channel Index of channel
*   @retval bool True if channel soloed
*/
bool getSolo(int channel);

/** @brief  Set mixer channel solo state
*   @param  channel Index of channel
*   @param  state True to solo channel
*   @note   Solo state is  accumulative, i.e. may solo several channels
*   @note   Solo main mixbus will disable all channel solo*/
void setSolo(int channel, bool value);

/** @brief  Get mixer channel mono state
*   @param  channel Index of channel
*   @retval bool True if channel monoed
*/
bool getMono(int channel);

/** @brief  Set mixer channel mono state
*   @param  channel Index of channel
*   @param  state True to mono channel
*   @note   Mono channel will mix inputs to both (left & right) main mix bus legs
*/
void setMono(int channel, bool value);

/** @brief  Get mixer channel instantaneous audio level
*   @param  channel Index of channel
*   @param  leg 0 for left, 1 for right
*   @retval float Instantaneous audio level [0..-200dBFS]
*/
float getPeakLevel(int channel, int leg);

/** @brief  Get mixer channel peak hold audio level
*   @param  channel Index of channel
*   @param  leg 0 for left, 1 for right
*   @retval float Peak hold audio level [0..-200dBFS]
*/
float getPeakHold(int channel, int leg);

/** @brief  Register a callback for mixer state changes
*   @param  callback Pointer to callback function
*   @param  parameters Bitmask flag indication of parameters to monitor [0:Fader, 1:Mute, 2:Solo, 4:Mono, 8:Peak Audio, 16:Peak Hold] (Default: All)
*/
void registerMixer(void* callback, uint32_t parameters=ALL);

/** @brief  Unregister a callback for mixer state changes
*   @param  callback Pointer to callback function
*   @param  parmeters Bitmask flag indication of parameters to unregister [0:Fader, 1:Mute, 2:Solo, 4:Mono, 8:Peak Audio, 16:Peak Hold] (Default: All) (Default: All)
*/
void unregisterMixer(void* callback, uint32_t parameters=ALL);

/*  Chains
    A chain is a set of engines with audio and control signal interlinks
    Each chain with audio processing has an associated mixer channel
    Chains are identified by an integer index
    Chain has a rectangular grid / graph of virtual slots into which engines may be placed
    Empty slots are assumed to connect adjacent horizontal slot signals
    A special virtual engine joins adjacent vertical slot signals
*/

/** @brief  Get the maximum quantity of chains
*   @retval uint16_t Maximum quantity of channel strips
*   @note   Attempt to access higher chain index will fail silently
*/
uint16_t getMaxChain();

/** @brief  Set maximum quantity of chains
*   @param  max Maximum quantity of chains
*   @note   Chains and associated mixer strips with higher indicies will be removed
*/
void setMaxChains(uint16_t max);

/** @brief  Get quantity of chains
*   @retval uint16_t Quantity of chains defined in current snapshot
*/
uint16_t getChainCount();

/** @brief  Register for notification on change of quantity of chains
*   @param  callback Pointer to callback (void)
*/
void registerChainCount(void* callback);

/** @brief  Get chain name
*   @param  chain Index of chain
*   @retval string Name of chain
*/
string getChainName(uint16_t chain);

/** @brief  Set chain name
*   @param  chain Index of chain
*   @param  string Name of chain
*/
void setChainName(uint16_t chain, string name);

/** @brief  Get bitmask of MIDI virtual cables used by chain
*   @param  chain Index of chain
*   @retval uint16_t Bitmask of MIDI virtual cables enabled
*/
uint16_t getChainMidiCables(uint16_t chain);

/** @brief  Get bitmask of MIDI channels assigned to chain on a cable
*   @param  chain Index of chain
*   @param  cable MIDI virtual cable [0..15, 0xFF for all cables] (Default: 1)
*   @retval uint16_t Bitmask of MIDI channels enabled on this virtual cable
*/
uint16_t getChainMidiChannels(uint16_t chain, uint8_t cable=1);

/** @brief  Set chain MIDI channel for a virtual cable
*   @param  chain Index of chain
*   @param  channel MIDI channel [0..15, 0xFF to disable MIDI]
*   @param  cable MIDI virtual cable [0..15, 0xFF for all cables] (Default: 1)
*   @note   To disable a virtual cable, set its MIDI channel to 0xFF
*/
void setChainMidiChannel(uint16_t chain, int channel, uint8_t cable=1);

/** @brief  Set chain MIDI channels for a virtual cable
*   @param  chain Index of chain
*   @param  channels Bitmask of MIDI channels
*   @param  cable MIDI virtual cable [0..15, 0xFF for all cables] (Default: 1)
*   @note   To disable a virtual cable, set its MIDI channels to 0
*/
void setChainMidiChannels(uint16_t chain, uint16_t channels, uint8_t cable=1);

/** @brief  Get chain note range filter minimum note value
*   @param  chain Index of chain
*   @retval uint8_t MIDI note value of lowest note sent to chain
*/
uint8_t getChainNoteMin(uint16_t chain);

/** @brief  Get chain note range filter maximum note value
*   @param  chain Index of chain
*   @retval uint8_t MIDI note value of highest note sent to chain
*/
uint8_t getChainNoteMax(uint16_t chain);

/** @brief  Set chain note range filter minimum note value
*   @param  chain Index of chain
*   @param  min MIDI note value of lowest note passed to chain [0..127, -1 for no change]
*   @param  max MIDI note value of highest note passed to chain [0..127, -1 for no change]
*   @note   max should be greater or equal to min otherwise max is ignored
*/
void setChainNoteRange(uint16_t chain, int8_t min, int8_t max);

/** @brief  Get chain MIDI transpose
*   @param  chain Index of chain
*   @retval int8_t Transpose value in MIDI note steps [-127..127]
*/
int8_t getChainTranspose(uint16_t chain);

/** @brief  Set chain MIDI transpose
*   @param  chain Index of chain
*   @param  transpose MIDI notes to transpose [-127..127]
*/
void setChainTranspose(uint16_t chain, int8_t transpose);

/** @brief  Get quantity of engines in a chain
*   @param  chain Index of chain (ALL for all chains, i.e. all instantiated engines)
*   @retval uint32_t Quantity of engines in chain
*/
uint32_t getEngineCount(uint16_t chain);

/** @brief  Get quantity of rows in chain graph
*   @param  chain Index of chain
*   @retval uint8_t Quantity of rows in graphdefaults
*/
uint8_t getChainRows(uint16_t chain);

/** @brief  Get quantity of columns in chain graph
*   @param  chain Index of chain
*   @retval uint8_t Quantity of columns in graphdefaults
*/
uint8_t getChainColumns(uint16_t chain);

/** @brief  Get id of engine within a chain
*   @param  chain Index of chain
*   @param  row Vertical position of engine within chain graph
*   @param  col Horizontal position of engine within chain graph
*   @retval uint32_t Id of engine
*   @note   Id is chain << 16 | col << 8 | row
*   @note   Physical inputs and outputs use engine id PHY_IN and PHY_OUT
*/
uint32_t getEngine(uint16_t chain, uint8_t row, uint8_t column);

/** @brief  Remove an engine from a chain
*   @param  engine Id of engine
*   @note   Engine instance is destroyed
*/
void removeEngine(uint32_t engine);

/** @brief  Add an engine to a chain
*   @param  chain Index of chain
*   @param  row Vertical position of engine within chain graph
*   @param  col Horizontal position of engine within chain graph
*   @param  class Name of engine class being added
*   @retval uint32_t Id of engine or NOT_FOUND if engine cannot be instantiated
*   @note   Engine instance is instantiated with default parameters and connected to adjacent horizontal slots
*   @note   Replaces and destroys any existing engine at same location in graph
*   @note   Use special classes JOIN_INPUT, JOIN_OUTPUT, JOIN_BOTH to connect input / output of horizontally adjacent slots to vertically adjacent slots
*   @note   JOIN classes give hints to autorouter which may be overriden by direct audio/MIDI routing of individual signals
*/
uint32_t addEngine(uint16_t chain, uint8_t row, uint8_t column, string class);

/** @brief  Moves an engine to a new position in a chain
*   @param  engine Id of engine
*   @param  chain New chain id
*   @param  row New row position
*   @param  column New column
*/
void moveEngine(uint16_t chain, uint8_t row, uint8_t column);

/** @brief  Copies (clones) an engine to a new position in a chain
*   @param  engine Id of engine to copy
*   @param  chain New chain id
*   @param  row New row position
*   @param  column New column
*/
void copyEngine(uint16_t chain, uint8_t row, uint8_t column);

/** @brief  Register notification on chain change
*   @param  callback Pointer to callback (uint32_t bitmask)
*   @param  parameter Bitmask of parameters within chain to monitor [1:Engine (added, removed, moved), 2:Name, 4:MIDI channel, 8:Note range, 16:Transpose]
*/
void registerChain(void* callback, uint32_t parameter);

/** @brief  Unregister notification of chain change
*   @param  callback Pointer to callback to unregister
*   @param  parameters Bitmask of parameters within chain to monitor [1:Engine (added, removed, moved), 2:Name, 4:MIDI channel, 8:Note range, 16:Transpose]
*/
void unregisterChain(void* callback, uint32_t parameters);


/*  Engines
    Engines are instances of Engine Classes
    Each chain consists of zero or more engines
*/

/** @brief  Get the class of engine within chain 
*   @param  engine Id of engine
*   @retval string Class name of engine
*/
string getEngineClass(uint32_t engine);

/** @brief  Get quantity of control signals connected to engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of paramter
*   @retval uint32_t Quantity of control signals
*/
uint32_t getEngineParameterControls(uint32_t engine, string parameter);

/** @brief  Get type of control signal connected to engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of paramter
*   @param  control Index of control signal
*   @retval uint32_t Control signals type [0:None, 1:MIDI, 2:OSC]
*/
uint32_t getEngineParameterControlType(uint32_t engine, string parameter, uint32_t control);

/** @brief  Get MIDI channel for control assigned to engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of paramter
*   @param  control Index of control signal
*   @retval uint8_t MIDI channel [0..15]
*   @note   Only valid for control signal type MIDI
*   @see    getEngineParameterControlType
*/
uint16_t getEngineParameterMidiChannel(uint32_t engine, string parameter, uint32_t control); 

/** @brief  Get MIDI control assigned to engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of paramter
*   @param  control Index of control signal
*   @retval uint8_t MIDI continuous controller [0..127]
*   @note   Only valid for control signal type MIDI
*/
uint16_t getEngineParameterMidiControl(uint32_t engine, string parameter, uint32_t control);

//!@todo    Add ability to set range & scale to control signals
//!@todo    Add ability to configure control signal as relative or absolute controller

/** @brief  Assign MIDI CC to control an engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of parameter
*   @param  channel MIDI channel
*   @param  cc MIDI continuous controller
*   @param  cables Bitmask of MIDI virtual cables (Default: 1, 0 to unassign)
*   @note   Duplicate channel & cc will replace existing configuration
*/
void addEngineParameterMidiControl(uint32_t engine, string parameter, uint8_t channel, uint8_t cc, uint16_t cables=1);

/** @brief  Get analogue control voltage assigned to engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of paramter
*   @param  control Index of control signal
*   @retval uint32_t Id of control voltage
*   @note   Only valid for control signal type CV
*/
uint32_t getEngineParameterCV(uint32_t engine, string parameter, uint32_t control);

/** @brief  Assign analogue control voltage to control an engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of parameter
*   @param  cv Id of control voltage input
*   @note   Duplicate cv will replace existing configuration
*/
void addEngineParameterCV(uint32_t engine, string parameter, uint32_t cv);

/** @brief  Get switch assigned to engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of paramter
*   @param  control Index of control signal
*   @retval uint32_t Id of switch
*   @note   Only valid for control signal type SWITCH
*/
uint32_t getEngineParameterSwitch(uint32_t engine, string parameter, uint32_t control);

/** @brief  Assign switch to control an engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of parameter
*   @param  switch Id of switch input
*   @note   Duplicate switch will replace existing configuration
*/
void addEngineParameterSwitch(uint32_t engine, string parameter, uint32_t switch);

/** @brief  Get OSC path assigned to engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of paramter
*   @retval string OSC path
*   @note   Only valid for control signal type OSC
*/
uint16_t getEngineParameterOscPath(uint32_t engine, string parameter);

/** @brief  Remove control of an engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of parameter
*   @param  control Index of control
*   @note   This may change index of other controls assigned to parameter
*/
void removeEngineParameterControl(uint32_t engine, string parameter, uint32_t control);

/** @brief  Get the index of an engine's currently loaded preset
*   @param  engine Id of engine
*   @retval int Preset index or -1 if no preset loaded
*/
int getEnginePreset(uint32_t engine);

/** @brief  Get the index of the bank of an engine's currently loaded preset
*   @param  engine Id of engine
*   @retval int Bank index or -1 if no preset loaded or engine does not support banks
*/
int getEngineBank(uint32_t engine);

/** @brief  Request an engine loads / selects a preset
*   @param  engine Id of engine
*   @param  bank Index of bank (ignored if bank not supported or required)
*   @param  preset Index of preset
*/
void selectEnginePreset(uint32_t engine, int bank, int preset);

/** @brief  Add currently selected preset to engine class
*   @param  engine Id of engine containing current configuration
*   @param  bank Index of bank (ignored if class does not support banks)
*   @param  preset Index of preset (replaces existing preset)
*   @param  name Name of new preset
*   @note   The parameters and configuration of selected engine are used
*/
void storeEnginePreset(uint32_t engine, uint32_t bank, uint32_t preset, string name);

/** @brief  Check if engine parameters differ from currently loaded preset
*   @brief  engine Id of engine
*   @retval bool True if preset modified
*/
bool isEngineModified(uint32_t engine);

/** @brief  Get an engine parameter value
*   @param  engine Id of engine
*   @param  parameter Name of parameter
*   @retval float Parameter value (zero if conversion from naitive data type fails)
*/
float getEngineParameterAsFloat(uint32_t engine, string parameter);

/** @brief  Set an engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of parameter
*   @param  value Value of parameter
*   @note   No change if conversion to naitive data type fails
*/
void setEngineParameterAsFloat(uint32_t engine, string parameter, float value);

/** @brief  Get an engine parameter value
*   @param  engine Id of engine
*   @param  parameter Name of parameter
*   @retval int Parameter value (zero if conversion from naitive data type fails)
*/
int getEngineParameterAsInt(uint32_t engine, string parameter);

/** @brief  Set an engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of parameter
*   @param  value Value of parameter
*   @note   No change if conversion to naitive data type fails
*/
void setEngineParameterAsInt(uint32_t engine, string parameter, int value);

/** @brief  Get an engine parameter value
*   @param  engine Id of engine
*   @param  parameter Name of parameter
*   @retval string Parameter value (empty string if conversion from naitive data type fails)
*/
string getEngineParameterAsString(uint32_t engine, string parameter);

/** @brief  Set an engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of parameter
*   @param  value Value of parameter
*   @note   No change if conversion to naitive data type fails
*/
void setEngineParameterAsString(uint32_t engine, string parameter, string value);

/** @brief  Get the chain an engine belongs to
*   @param  engine Id of engine
*   @retval uint16_t Id of chain or 0xFFFF for invalid engine id
*/
uint16_t getEngineChain(uint32_t engine);

/** @brief  Get chain row an engine is positioned
*   @param  engine Id of engine
*   @retval uint8_t Row or 0xFF for invalid engine id
*/
uint8_t getEngineRow(uint32_t engine);

/** @brief  Get chain column an engine is positioned
*   @param  engine Id of engine
*   @retval uint8_t Column or 0xFF for invalid engine id
*/
uint8_t getEngineColumn(uint32_t engine);

/** @brief  Register for engine preset change
*   @param  callback Pointer to callback (uint32_t engine, uint32_t bank, uint32_t preset)
*/
void registerEnginePreset(void* callback);

/** @brief  Unregister for engine preset change
*   @param  callback Pointer to callback (uint32_t engine, uint32_t bank, uint32_t preset)
*/
void unregisterEnginePreset(void* callback);

/** @brief  Register for engine parameter change
*   @param  callback Pointer to callback (uint32_t engine, uint32_t parameter)
*   @param  parameter Index of parameter to monitor
*/
void registerEngineParameter(void* callback, uint32_t parameter);

/** @brief  Unregister for engine parameter change
*   @param  callback Pointer to callback
*   @param  parameter Index of parameter to monitor
*/
void unregisterEngineParameter(void* callback, uint32_t parameter);


/*  Engine Classes
    Classes or types of different engines
    May be audio or MIDI (or other control signal) generators
    May be audio or MIDI (or other control signal) effects or processors
*/

/** @brief  Get quantity of engine classes
*   @retval uint32_t Quantity of supported engine classes
*/
uint32_t getEngineClassCount();

/** @brief  Get name of engine class
*   @param  index Index of engine class
*   @retval string Name of engine class
*   @note   Allows iteration to detect class names
*/
string getEngineClass(int index);

/** @brief  Get engine class type
*   @param  class Name of engine class
*   @retval string Engine class type [Audio effect | MIDI effect | Audio generator | etc. Should this be an enumeration integer?]
*/
string getEngineClassType(string class); 

/** @brief  Get quantity of signal inputs of an engine class
*   @param  class Name of engine class
*   @retval uint8_t Quantity of signal inputs
*/
uint8_t getEngineClassInputCount(string class);

/** @brief  Get quantity of signal outputs of an engine class
*   @param  class Name of engine class
*   @retval uint8_t Quantity of signal outputs
*/
uint8_t getEngineClassOutputCount(string class);

/** @brief  Get quantity of banks available to an engine class
*   @param  class Name of engine class
*   @retval uint32_t Quantity of banks
*/
uint32_t getEngineClassBankCount(string class);

/** @brief  Get name of an engine class bank
*   @param  class Name of engine class
*   @param  bank Index of bank
*   @retval string Name of bank
*/
string getEngineClassBankName(string class, uint32_t bank);

/** @brief  Set name of an engine class bank
*   @param  class Name of engine class
*   @param  bank Index of bank
*   @param  name New name for bank
*/
void setEngineClassBankName(string class, uint32_t bank, string name);

/** @brief  Add a bank to an engine class
*   @param  class Name of engine class
*   @param  name Name of new bank
*/
void addEngineClassBank(string class, string name);

/** @brief  Remove a bank from an engine class
*   @param  class Name of engine class
*   @param  name Name of bank to remove
*   @note   Presets within bank are destroyed
*/
void removeEngineClassBank(string class, string name);

/** @brief  Get quantity of presets within an engine class bank
*   @param  class Name of engine class
*   @param  bank Index of bank
*   @retval uint32_t Quantity of presets
*/
uint32_t getEngineClassPresetCount(string class, uint32_t bank);

/** @brief  Get name of an engine class preset
*   @param  class Name of engine class
*   @param  bank Index of bank (ignored if class does not support banks)
*   @param  preset Index of preset
*   @retval string Name of class or empty string if preset does not exist
*/
string getEngineClassPresetName(string class, uint32_t bank, uint32_t preset);

/** @brief  Removes preset from engine class
*   @param  class Name of engine class
*   @param  bank Index of bank (ignored if class does not support banks)
*   @param  preset Index of preset (replaces existing preset)
*/
void removeEngineClassPreset(string class, uint32_t bank, uint32_t preset);

/** @brief  Get quantity of favourite presets within an engine class bank
*   @param  class Name of engine class (may be empty to select all classes)
*   @retval uint32_t Quantity of presets
*/
uint32_t getFavouritePresetCount(string class, uint32_t bank);

/** @brief  Add preset to favourites
*   @param  class Name of engine class
*   @param  bank Index of bank (ignored if class does not support banks)
*   @param  preset Index of preset
*/
void addFavouritePreset(string class, uint32_t bank, uint32_t preset);

/** @brief  Remove preset from favourites
*   @param  class Name of engine class
*   @param  bank Index of bank (ignored if class does not support banks)
*   @param  preset Index of preset
*/
void removeFavouritePreset(string class, uint32_t bank, uint32_t preset);

/** @brief  Get favourte preset class
*   @param  favourite Index of favourite
*   @retval string Name of class or empty string if favourite does not exist
*/
string getFavouriteClass(uint32_t favourite);

/** @brief  Get favourte preset bank
*   @param  favourite Index of favourite
*   @retval uint32_t Index of bank within which favourite resides
*/
uint32_t getFavouriteBank(uint32_t favourite);

/** @brief  Get favourte preset preset
*   @param  favourite Index of favourite
*   @retval uint32_t Index of preset
*/
uint32_t getFavouritePreset(uint32_t favourite);

/** @brief  Check if a preset is a favourite
*   @param  class Name of engine class
*   @param  bank Index of bank (ignored if class does not support banks)
*   @param  preset Index of preset
*   @retval bool True if preset is a favourite
*/
bool isFavourite(string class, uint32_t bank, uint32_t preset);

/** @brief  Get quantity of parameters for an engine class
*   @param  class Name of engine class
*   @retval uint32_t Quantity of parameters the engine class exposes
*/
uint32_t getEngineClassParameterCount(string class);

/** @brief  Get an engine class parameter name
*   @param  class Name of engine class
*   @param  parameter Index of the parameter
*   @retval string Parameter name
*/
string getEngineClassParameterName(string class, uint32_t parameter);

/** @brief  Get an engine class parameter type
*   @param  class Name of engine class
*   @param  parameter Index of the parameter
*   @retval string Parameter type
*/
string getEngineClassParameterType(string class, uint32_t parameter);

/** @brief  Get an engine class parameter minimum value
*   @param  class Name of engine class
*   @param  parameter Index of the parameter
*   @retval float Minimum value (0 if not valid)
*/
float getEngineClassParameterMinimum(string class, uint32_t parameter);

/** @brief  Get an engine class parameter maximum value
*   @param  class Name of engine class
*   @param  parameter Index of the parameter
*   @retval float Maximum value (0 if not valid)
*/
float getEngineClassParameterMaximum(string class, uint32_t parameter);

/** @brief  Get size of step a class parameter value may change by
*   @param  class Name of engine class
*   @param  parameter Index of the parameter
*   @retval float Parameter step size (0.0 if not valid)
*/
float getEngineClassParameterStep(string class, uint32_t parameter);

/** @brief  Get class parameter units
*   @param  class Name of engine class
*   @param  parameter Index of the parameter
*   @retval string Parameter units (empty string if not valid)
*/
float getEngineClassParameterUnits(string class, uint32_t parameter);

/** @brief  Get class parameter group
*   @param  class Name of engine class
*   @param  parameter Index of the parameter
*   @retval string Name of group parameter belongs (empty string if not valid)
*/
float getEngineClassParameterGroup(string class, uint32_t parameter);

/** @brief  Get quantity of class parameter enumeration values
*   @param  class Name of engine class
*   @param  parameter Index of the parameter
*   @retval uint32_t Quantity of enumeration values (0 if not valid)
*/
uint32_t getEngineClassParameterEnums(string class, uint32_t parameter);

/** @brief  Get class parameter enumeration name
*   @param  class Name of engine class
*   @param  parameter Index of the parameter
*   @param  enum Index of the enumeration
*   @retval string Name of the indexed enumeration (empty string if not valid)
*/
string getEngineClassParameterEnumName(string class, uint32_t parameter, uint32_t enum);

/** @brief  Get class parameter enumeration value as string
*   @param  class Name of engine class
*   @param  parameter Index of the parameter
*   @param  enum Index of the enumeration
*   @retval string value of the indexed enumeration (empty string if not valid or cannot convert type)
*/
string getEngineClassParameterEnumString(string class, uint32_t parameter, uint32_t enum);

/** @brief  Get class parameter enumeration value as float
*   @param  class Name of engine class
*   @param  parameter Index of the parameter
*   @param  enum Index of the enumeration
*   @retval float value of the indexed enumeration (0.0 if not valid or cannot convert type)
*/
float getEngineClassParameterEnumFloat(string class, uint32_t parameter, uint32_t enum);

/** @brief  Get class parameter enumeration value as integer
*   @param  class Name of engine class
*   @param  parameter Index of the parameter
*   @param  enum Index of the enumeration
*   @retval int value of the indexed enumeration (0 if not valid or cannot convert type)
*   @todo   Do we need to get integer value? Could just get float and cast to integer
*/
int getEngineClassParameterEnumInt(string class, uint32_t parameter, uint32_t enum);

/*  Routing Graph
    Audio and MIDI routing is handled by jack
    CV routing is handled by Zynthian core
    Quantity of engines can be obtained by calling getEngines(ALL)
    Quantity of engine inputs can be obtained by calling getEngineClassInputCount()
    Quantity of engine outputs can be obtained by calling getEngineClassOutputCount()
    Exposed via API to allow CRUD on jack and CV routing by clients
    Manipulation of routing graph overrides chain automatic routing
*/

/** @brief  Get quantity of routes / interconnects in routing graph
*   @param  types Bitmask of signal type [1:Audio, 2:MIDI, 3:CV]
*   @retval uint32_t Quantity of routes in graph
*/
uint32_t getGraphRoutes(uint32_t types=ALL);

/** @brief  Get engine connected to route
*   @param  route Index of route
*   @param  destination True if node is destination of route
*   @param  types Bitmask of signal type [1:Audio, 2:MIDI, 3:CV] (Default: ALL)
*   @retval uint32_t Id of engine (NOT_FOUND if route id is invalid)
*   @note   Can iterate over same index range provided by getGraphRoutes with same types
*/
uint32_t getGraphEngine(uint32_t route, bool destination, uint32_t types=ALL);

/** @brief  Add an interconnect to routing graph
*   @param  source Id of source engine
*   @param  output Index of source engine output
*   @param  destination Id of destination engine
*   @param  input Index of destination engine input
*   @retval bool True on success
*   @note   Physical inputs and outputs use engine id PHY_IN and PHY_OUT
*/
bool addRoute(uint32_t source, uint32_t output, uint32_t destination, uint32_t input);

/** @brief  Remove an interconnect from routing graph
*   @param  route Index of route
*   @param  types Bitmask of signal type [1:Audio, 2:MIDI, 3:CV] (Default: ALL)
*   @note   Index is relevant to types
*/
void removeRoute(uint32_t route, uint32_t types=ALL);

/** @brief  Remove an interconnect from routing graph
*   @param  source Id of source engine
*   @param  output Index of source engine output
*   @param  destination Id of destination engine
*   @param  input Index of destination engine input
*/
void removeRoute(uint32_t source, uint32_t output, uint32_t destination, uint32_t input);

/** @brief  Register for route change
*   @param  callback Pointer to callback function (uint32_t route)
*/
void registerRoute(void* callback);

/** @brief  Unregister for route change
*   @param  callback Pointer to callback function
*/
void unregisterRoute(void* callback);


/*  Presets
    A preset is a configuration of an engine
    Preset access is implemented in Engine and Engine Class sections
*/


/*  Snapshots
    A snapshot is a full capture of the whole data model including:
    * instantiated engines
    * engine parameters
    * routing
    * mixer settings
    * chain configuration
    * etc.
*/

/** @brief  Get quantity of available snapshots
*   @retval uint32_t Quantity of available snapshots
*/
uint32_t getSnapshotCount();

/** @brief  Get name of snapshot
*   @param  snapshot Index of snapshot
*   @retval string Name of snapshot
*/
string getSnapshotNameByIndex(uint32_t snapshot);

/** @brief  Get name of snapshot
*   @param  path Full path and filename of snapshot
*   @retval string Name of snapshot
*/
string getSnapshotNameByPath(string path);

/** @brief  Set name of currently loaded snapshot
*   @param  snapshot Index of snapshot
*   @param  name New name for snapshot
*/
void setSnapshotName(uint32_t snapshot, string name);

/** @brief  Restore a snapshot from persistent storage
*   @param  path Full path and filename to snapshot file
*   @retval bool True on success
*/
bool loadSnapshot(string path);

/** @brief  Store current data model as a snapshot to persistent storage
*   @param  path Full path and filename to snapshot file
*   @retval bool True on success
*/
bool saveSnapshot(string path);


/*  Physical UI
    Access to switches, encoders, endless pots, LEDs, etc.
*/

/** @brief  Get quantity of switches
*   @retval uint32_t Quantity of switches
*/
uint32_t getSwitchCount();

/** @brief  Get switch state
*   @param  switch Index of switch
*   @retval bool True if switch closed
*   @todo   Rationalise read of Boolean values - getState or isClosed?
*/
bool isSwitchClosed(uint32_t switch);

/** @brief  Register switch change
*   @param  callback Pointer to callback function (uint32_t switch, bool state)
*   @param  switch Index of switch (Default: ALL)
*/
void registerSwitch(void* callback, uint32_t switch=ALL);

/** @brief  Unregister switch change
*   @param  callback Pointer to callback function
*   @param  switch Index of switch (Default: ALL)
*/
void unregisterSwitch(void* callback, uint32_t switch=ALL);

/** @brief  Assign a MIDI command to a UI switch
*   @param  switch Index of switch
*   @param  event Type of MIDI event to send when switch closed
*   @param  channel MIDI channel
*   @param  command MIDI command
*   @param  value MIDI value
*   @retval int
*   @todo   Document return value. What is event type?
*   @todo   Where does this MIDI command go? Do we need to route to an output?
*/
int assignSwitchMidi(uint8_t switch, midi_event_type event, uint8_t midiChannel, uint8_t command, uint8_t value);

//!@todo Document get_zynswitch
unsigned int getZynswitch(uint8_t switch, unsigned int longDtus);

/** @brief  Get quantity of rotary encoders
*   @retval uint32_t Quantity of rotary encoders
*   @note   Endless potentiometers and rotary encoders are exposed as zynpot
*/
uint32_t getZynpotCount();

/** @brief  Configure a zynpot
*   @param  zynpot Index of zynpot
*   @param  min Minimum value
*   @param  max Maximum value
*   @param  value Current / default value
*   @param  step Quantity of units to skip for each incremental change
*   @retval int
*   @todo   Describe return value
*/
int zynpotSetup(uint8_t zynpot, int32_t min, int32_t max, int32_t value, int32_t step);

/** @brief  Get current value of zynpot
*   @param  zynpot Index of zynpot
*   @retval int32_t Current value
*/
int32_t zynpotGetValue(uint8_t zynpot);

/** @brief  Set value of zynpot
*   @param  zynpot Index of zynpot
*   @param  value New value
*   @param  send True to send notification of new value
*   @todo   Describe return value
*/
int zynpotSetValue(uint8_t zynpot, int32_t value, bool send);

/** @brief  Register zynpot change
*   @param  callback Pointer to callback function (uint32_t zynpot, int value)
*   @param  zynpot Index of zynpot (Default: ALL)
*/
void registerZynpot(void* callback, uint32_t zynpot = ALL);

/** @brief  Unregister zynpot change
*   @param  callback Pointer to callback function
*   @param  zynpot Index of zynpot (Default: ALL)
*/
void unregisterZynpot(void* callback, uint32_t zynpot = ALL);

//!@todo   Document getZynpotFlag
uint8_t get_value_flag_zynpot(uint8_t zynpot);

/** @brief  Assign MIDI command to zynpot
*   @param  zynpot Index of zynpot
*   @param  channel MIDI channel
*   @param  command MIDI command
*   @todo   Describe return value
*/
int zynpotSetupMidi(uint8_t zynpot, uint8_t channel, uint8_t command);

/** @brief  Assign OSC command to zynpot
*   @param  zynpot Index of zynpot
*   @param  path OSC path
*   @todo   Describe return value
*/
int zynpotSetupOsc(uint8_t zynpot, char *path);

/** @brief  Register arbitrary OSC path
*   @param  callback Pointer to callback function (string path, ...)
*   @param  path OSC path
*   @param  parameters Comma separated list of OSC parameter types
*   @todo   How to pass values to callback?
*/
void registerOsc(void* callback, string path, string parameters);

/** @brief  Unregister arbitrary OSC path
*   @param  callback Pointer to callback function
*   @param  path OSC path
*/
void unregisterOsc(void* callback, string path);

/*  Step Sequencer
    See zynseq.h
*/


/*  Real time messages
    Messages sent with low latency
*/

/** @brief  Send a MIDI command
*   @param  channel MIDI channel
*   @param  command MIDI command
*   @param  value MIDI value (ignored for 2 byte commands)
*/
void sendMidi(uint8_t channel, uint8_t command, uint8_t value);

/** @brief  Register callback to receive MIDI messages
*   @param  callback Pointer to callback (uint8_t channel, uint8_t command, uint8_t value)
*   @param  channel MIDI channel (0..15, 0xFF for all)
*   @param  command MIDI command (0..127, 0xFF for all)
*   @param  min Minimum MIDI value (0..127, Default: 0, ignored for 2 byte commands)
*   @param  max Maximum MIDI value (0..127, Default: 127, ignored for 2 byte commands)
*/
void registerMidi(void* callback, uint8_t channel, uint8_t command, uint8_t min=0, uint8_t max=127);

/** @brief  Unregister callback to receive MIDI messages
*   @param  callback Pointer to callback
*   @param  channel MIDI channel (0..15, 0xFF for all)
*   @param  command MIDI command (0..127, 0xFF for all)
*/
void unregisterMidi(void* callback, uint8_t channel, uint8_t command);

/** @brief  Get transport state
*   @retval uint8_t Current transport state
*/
uint8_t getTransportState();

/** @brief  Set transport state
*   @param  state New transport state [STOPPED | ROLLING]
*/
void setTransportState(uint8_t state);

/** @brief  Register transport state change
*   @param  callback Pointer to callback funtion (uint8_t state)
*/
void registerTransportState(void* callback);

/** @brief  Unregister transport state change
*   @param  callback Pointer to callback funtion.
*/
void unregisterTransportState(void* callback);

/** @brief  Get transport position
*   @retval uint32_t Transport position in ticks
*/
uint32_t getTransportPosition();

/** @brief  Set transport position
*   @param  position New position in ticks
*/
void setTransportPosition(uint32_t position);

/** @brief  Register transport position change
*   @param  callback Pointer to callback funtion (uint32_t position)
*   @param  delta Change in ticks before new postion triggers callback
*/
void registerTransportPosition(void* callback, uint32_t delta);

/** @brief  Unregister transport position change
*   @param  callback Pointer to callback funtion
*/
void unregisterTransportPosition(void* callback);


/*  System messages
    Control and monitoring of core system
*/

/** @brief  Get time in seconds since boot
*   @retval uint32_t Quantity of seconds since boot
*/
uint32_t getUptime();

/** @brief  Get quantity of xruns since last reset
*   @retval uint32_t Quantity of xruns
*/
uint32_t getXruns();

/** @brief  Reset xrun counter
*/
void resetXruns();

/** @brief  Get quantity of high temperature alerts since last reset
*   @retval uint32_t Quantity of high temperature alerts
*/
uint32_t getHigTemperature()

/** @brief  Reset over voltage alert counter
*/
void resetHighTemperature();

/** @brief  Get quantity of under voltage alerts since last reset
*   @retval uint32_t Quantity of under voltage alerts
*/
uint32_t getUnderVoltage()

/** @brief  Reset under voltage alert counter
*/
void resetUnderVoltage();

/** @brief  Register warning
*   @param  callback Pointer to callback (uint32_t warning)
*   @param  warning Bitmask of wanring types [1:xrun, 2:Temperature, 4:Voltage]
*/
void registerWarning(void* callback, uint32_t warning);

/** @brief  Unregister warning
*   @param  callback Pointer to callback
*   @param  warning Bitmask of wanring types [1:xrun, 2:Temperature, 4:Voltage]
*/
void unregisterWarning(void* callback, uint32_t warning);

/** @brief  Restart core
*   @note   Engines will be destroyed and recreated. Sequences will be stopped.
*/
void restartCore();

/** @brief  Shutdown and power off device
*/
void shutdown();

/** @brief  Restart device
*/
void reboot();

/** @brief  Send all note off message to all engines
*/
void panic();

/** @brief  Start recorder
*   @param  type Recording type [1:Audio recording, 2:Audio playback, 4:MIDI recording, 8:MIDI playback]
*   @param  filename Full path and filename for new recording (Default: Unique timestamped filename)
*/
void startRecorder(uint8_t type, string filename="");

/** @brief  Stop recorder
*   @param  type Bitmask of recording type [1:Audio recording, 2:Audio playback, 4:MIDI recording, 8:MIDI playback]
*   @param  filename Full path and filename of recording (Default: All)
*/
void stopRecorder(uint8_t type, string filename="");

/** @brief  Check if any recording or playback running
*   @param  type Bitmask of recording type [1:Audio recording, 2:Audio playback, 4:MIDI recording, 8:MIDI playback]
*   @retval bool True if any specified recorder is rolling
*/
bool isRecorderRolling(uint8_t type);

/** @brief  Register recorder
*   @param  callback Pointer to callback (uint8_t type, string filename)
*   @param  type Bitmask of recording type [1:Audio recording, 2:Audio playback, 4:MIDI recording, 8:MIDI playback]
*/
void registerRecorder(void* callback, uint8_t type);

/** @brief  Unregister recorder
*   @param  callback Pointer to callback
*   @param  type Bitmask of recording type [1:Audio, 2:MIDI]
*/
void unregisterRecorder(void* callback, uint8_t type);
