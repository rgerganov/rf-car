#ifndef __RF_H__
#define __RF_H__
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <atomic>

enum Direction
{
    FWD, FWD_LEFT, FWD_RIGHT, BACK, BACK_LEFT, BACK_RIGHT, LEFT, RIGHT, NONE, SYNC, STOP
};

struct hackrf_device;
class RfCar
{
public:
    RfCar(uint64_t freq, int sample_rate, int symbol_rate, bool always_tx) : freq(freq),
                                                                             sample_rate(sample_rate),
                                                                             symbol_rate(symbol_rate),
                                                                             always_tx(always_tx) {
    }
    bool init();
    void startTx();
    void changeState(Direction dir, int gain_tx);
    void stopTx();
    void close();
    // return if the specified direction is supported by the implementation
    virtual bool supportDirection(Direction dir) = 0;
    // send specific pattern at the end of the transmission
    virtual void txEnd() = 0;
    virtual int txCallback(uint8_t* buffer, int valid_length) = 0;
    virtual ~RfCar() {}

protected:
    uint64_t freq;
    int sample_rate;
    int symbol_rate;
    int last_gain_tx = 30;
    std::atomic<Direction> last_dir{NONE};
    hackrf_device *device;
    // current index in the bit pattern being transmitted
    // always divided by samples_per_bit and used modulo pattern size
    std::atomic<uint32_t> pos{0};
    // true if tx has been started
    bool tx_started = false;
    // always transmit, even if the user is not pressing anything
    // (the app is more responsive this way because start/stop tx takes time)
    bool always_tx;
};

class OokCar : public RfCar
{
public:
    OokCar(uint64_t freq, int sample_rate, int symbol_rate, bool always_tx);
    virtual bool supportDirection(Direction dir) {
        return true;
    }
    virtual void txEnd() {
        // nop
    }
    void invertSteering();
    void invertThrottle();
    virtual int txCallback(uint8_t* buffer, int valid_length);
    virtual ~OokCar() {}

private:
    std::vector<int> patterns[11];
    std::vector<float> filter;
};

class FskCar : public RfCar
{
public:
    FskCar(uint64_t freq, int sample_rate, int symbol_rate, bool always_tx);
    void sendSync();
    virtual bool supportDirection(Direction dir) {
        return dir != LEFT && dir != RIGHT;
    }
    virtual void txEnd();
    virtual int txCallback(uint8_t* buffer, int valid_length);
    virtual ~FskCar() {}

private:
    // use maps as sparse arrays mapping array index to array value
    std::unordered_map<int, int> patterns[11];
    int pattern_size = 0;
    int sync_pattern_size = 0;
    float phase = 0;
};

#endif