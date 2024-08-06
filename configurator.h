#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

// Make header for defines
#define POW_CONF_ADDR "0x83c03044"    // Address for pow configuration
#define ON_OFF_ADDR   "0x83c03040 "   // Address for on/off and work.period

#define POW_CONF_DATA "0x00000004"  // Data to conf pow step
#define ON_OFF_DATA   "0x00100400"  // Data to on/off and work.period

#define MAP_ADDR      0x83c02000    // First address for reading

#define START_ADDR    0x83c03000    // First address for writing

#define AMP_BIT_LEN 8               // Size of the First part in writing data
#define PH_INCR_BIT_LEN 12          // Middle and Last size of parts in writing data
#define CONC_BIT_LEN 32             // Full size of writing data

// Constants from DDS
#define F_CLK 200000000             // Frequency of zc-702
#define BITS 16                     // Number of Bits in phase accumulator (Phase width) and phase increment value

// Constants from devmem2
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)