// To compile this simple example:
// $ gcc -o quick_start_example1 quick_start_example1.c -lnfc

#include <cstdlib>
#include <iostream>
#include <vector>

#include <nfc/nfc.h>

int main(int argc, const char *argv[]) {

  // Allocate only a pointer to nfc_context
  nfc_context *context;

  // Initialize libnfc and set the nfc_context
  nfc_init(&context);
  if (context == nullptr) {
    std::cerr << "failed: nfc_init" << std::endl;
    return 1;
  }

#if 1
  std::vector<nfc_connstring> connstrings(8);
  size_t num_devices = nfc_list_devices(context, connstrings.data(), 8);
  if (num_devices == 0) {
    std::cout << "no nfc devices:" << std::endl;
    return 1;
  }
  for (size_t i = 0; i < num_devices; ++i) {
    std::cout << "device" << i << ": " << connstrings[i] << std::endl;
  }
  std::cout << "select device0" << std::endl;
  nfc_device *pnd = nfc_open(context, connstrings[0]);
#else
  // Open, using the first available NFC device which can be in order of
  // selection:
  //   - default device specified using environment variable or
  //   - first specified device in libnfc.conf (/etc/nfc) or
  //   - first specified device in device-configuration directory
  //   (/etc/nfc/devices.d) or
  //   - first auto-detected (if feature is not disabled in libnfc.conf) device
  nfc_device *pnd = nfc_open(context, nullptr);
#endif

  if (pnd == nullptr) {
    std::cerr << "failed: nfc_open: Unable to open NFC device." << std::endl;
    return 1;
  }

  // Set opened NFC device to initiator mode
  if (nfc_initiator_init(pnd) < 0) {
    std::cerr << "failed: nfc_initiator_init: " << nfc_strerror(pnd)
              << std::endl;
    return 1;
  }

  std::cout << "device name = " << nfc_device_get_name(pnd) << std::endl;

  std::vector<nfc_target> nts(8);
  // Poll for a ISO14443A (MIFARE) tag
  const nfc_modulation nm_mifare = {
      .nmt = NMT_ISO14443A,
      .nbr = NBR_106,
  };

  int num_targets =
      nfc_initiator_list_passive_targets(pnd, nm_mifare, nts.data(), 8);
  if (num_targets < 0) {
    std::cerr << "failed: nfc_initiator_list_passive_targets: "
              << nfc_strerror(pnd) << std::endl;
    return 1;
  }
  std::cout << "ISO14443A tag was found: " << num_targets << std::endl;
  for (int i = 0; i < num_targets; ++i) {
    // str_nfc_modulation_type
    std::cout << "#" << i << std::endl;
    std::vector<char *> bufp(16);
    int n = str_nfc_target(bufp.data(), &nts[i], true);
    for (int j = 0; j < n; ++j) {
      std::cout << bufp[j] << std::endl;
      nfc_free(bufp[j]);
    }
  }

  // Close NFC device
  nfc_close(pnd);
  // Release the context
  nfc_exit(context);
  exit(EXIT_SUCCESS);
}
