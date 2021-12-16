/*  Zynthian core API */

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
float getPeakProgramme(int channel, int leg);

/** @brief  Get mixer channel peak hold audio level
*   @param  channel Index of channel
*   @param  leg 0 for left, 1 for right
*   @retval float Peak hold audio level [0..-200dBFS]
*/
float getPeakHoldA(int channel, int leg);

/** @brief  Register a callback for mixer state changes
*   @param  callback Pointer to callback function
*   @param  bitmap Bitmap flag indication of parameters to monitor
*/
void registerMixer(void* callback, uint32_t bitmap);

/** @brief  Unregister a callback for mixer state changes
*   @param  callback Pointer to callback function
*   @param  bitmap Bitmap flag indication of parameters to unregister (Default: 0xFFFFFFFF unregister all)
*/
void unregisterMixer(void* callback, uint32_t bitmap=0xFFFFFFFF);

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

/** @brief  Get chain MIDI channel
*   @param  chain Index of chain
*   @retval int MIDI channel [0..15, -1 for none]
*/
int getChainMidiChannel(uint16_t chain);

/** @brief  Set chain MIDI channel
*   @param  chain Index of chain
*   @param  channel MIDI channel [0..16, -1 to disable MIDI]
*/
void setChainMidiChannel(uint16_t chain, int channel);

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
*   @param  chain Index of chain
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
*   @retval uint32_t Index of engine or 0xFFFFFFFF if engine cannot be instantiated
*   @note   Engine instance is instantiated with default parameters and connected to adjacent horizontal slots
*   @note   Replaces and destroys any existing engine at same location in graph
*   @note   Use special classes JOIN_INPUT, JOIN_OUTPUT, JOIN_BOTH to connect input / output of horizontally adjacent slots to vertically adjacent slots
*   @note   JOIN classes give hints to autorouter which may be overriden by direct audio/MIDI routing of individual signals
*/
uint32_t addEngine(int chain, int row, int column, string class);

/*  Engines
    Engines are instances of Engine Classes
    Each chain consists of zero or more engines
*/

/** @brief  Get the class of engine within chain 
*   @param  chain Index of chain
*   @param  row Vertical position of engine within chain graph
*   @param  col Horizontal position of engine within chain graph
*   @retval string Class name of engine
*/
string getEngineClass(int chain, int row, int column);

/** @brief  Get MIDI control assigned to engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of paramter
*   @retval uint16_t MSB:MIDI channel, LSB: MIDI CC
*/
uint16_t getEngineMidiControl(int engine, string parameter); 

/** @brief  Assigns a MIDI CC to control an engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of parameter
*   @param  uint8_t channel MIDI channel
*   @param  uint8_t cc MIDI continuous controller
*/
void addEngineMidiControl(uint32_t engine, string parameter, int channel, int cc);

/** @brief  Unassigns a MIDI CC from controlling an engine parameter
*   @param  engine Id of engine
*   @param  parameter Name of parameter ("ALL" to remove all parameters)
*   @param  channel MIDI channel [0..15, 0xFF for all channels]
*   @param  cc MIDI continuous controller [0..127, 0xFF for all controllers]
/
void removeEngineMidiControl(uint32_t engine, string parameter, int channel, int cc); 

int getEnginePreset(uint32_t engine);
int getEngineBank(uint32_t engine);
void setEnginePreset(uint32_t engine, int bank, int preset);
bool isEngineModified(uint32_t engine);
template getEngineParameterValue(uint32_t engine, string parameter); //Return type depends on parameter type - do we implement getter for each data type, converting value as well as we can?
void setEngineParamterValue(uint32_t engine, string parameter, template value); //Value type depends on parameter - do we implement setter for each data type?
int getEngineChain(int engine); //This is only required if we have a reference for each engine rather than access via chain,row,columm.

// Engine Classes
int getEngineClassCount();
string getEngineClass(int index); //Allows iteration to detect class names which are used to index classes
string getEngineClassType(string class); //Audio effect / MIDI effect / Audio generator / etc. Sould this be an enumeration integer?
int getEngineClassInputCount(string class);
int getEngineClassOutputCount(string class);
int getEngineClassBankCount(string class);
string getEngineClassBankName(string class, int bank);
void setEngineClassBankName(string class, int bank, string name);
void addEngineClassBank(string class, string name);
void removeEngineClassBank(string class, string name);
int getEngineClassPresetCount(string class, int bank);
string getEngineClassPresetName(string class, int bank, int preset);