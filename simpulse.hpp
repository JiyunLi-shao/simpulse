//
// simpulse: a library for simulating pulses in radio astronomy.  
//
// Right now there isn't much here, just a class representing a single channelized, dispersed, 
// scattered pulse.  In the future I may add more features (e.g. pulsars and scintillated pulses).
//

#ifndef _SIMPULSE_HPP
#define _SIMPULSE_HPP

#if (__cplusplus < 201103) && !defined(__GXX_EXPERIMENTAL_CXX0X__)
#error "This source file needs to be compiled with C++0x support (g++ -std=c++0x)"
#endif

#include <iostream>
#include <string>
#include <vector>

namespace simpulse {
#if 0
}; // pacify emacs c-mode
#endif


// Returns dispersion delay in seconds
// The 'DM' parameter is the dispersion measure in its standard units (pc cm^{-3})
inline double dispersion_delay(double dm, double freq_MHz)
{
    return 4.148806e3 * dm / (freq_MHz * freq_MHz);
}

// Returns scattering time in seconds
// We define the 'SM' to be the scattering time in _milliseconds_ (not seconds) at 1 GHz
inline double scattering_time(double sm, double freq_MHz)
{
    return 1.0e-3 * sm / pow(freq_MHz/1000.0, 4.4);
}

//
// struct single_pulse: represents one dispersed, scattered pulse, in a fixed frequency channelization.
//
struct single_pulse {
    int     pulse_nt;          // number of samples used "under the hood" to represent pulse (1024 is usually a good default)
    int     nfreq;             // number of frequency channels, assumed equally spaced
    double  freq_lo_MHz;       // lower limit of frequency band
    double  freq_hi_MHz;       // upper limit of frequency band

    double  dm;                      // dispersion measure in its standard units (pc cm^{-3})
    double  sm;                      // we define the "scattering measure" SM to be scattering time in milliseconds at 1 GHz
    double  intrinsic_width;         // frequency-independent Gaussian width in seconds
    double  fluence;                 // F(nu_0) = int I(t) dt evaluated at central frequency
    double  spectral_index;          // F(nu) = F(nu_0) (nu/nu_0)^alpha
    double  undispersed_arrival_time;  // arrival time at nu=infty, in seconds relative to an arbitrary origin.

    //
    // Under-the-hood representation of the pulse.
    // This shouldn't be needed from the outside world.
    // Note that all "internal" times are relative to undispersed_arrival_time.
    //
    std::vector<double> pulse_t0;        // start time of pulse in i-th channel, relative to undispersed_arrival_time
    std::vector<double> pulse_t1;        // end time of pulse in i-th channel, relative to undispersed_arrival_time
    std::vector<double> pulse_freq_wt;   // weight of pulse in i-th channel, determined by spectral index.
    std::vector<double> pulse_cumsum;    // shape (nfreq, pulse_nt+1) array, containing cumulative sum of pulse, normalized to sum=1
    double min_t0;                       // minimum of all pulse_t0 values
    double max_t1;                       // maximum of all pulse_t1 values
    double max_dt;                       // maximum of all (pulse_t1-pulse_t0) values
    
    //
    // Constructor arguments:
    //
    //   pulse_nt = number of samples used "under the hood" to represent the pulse (suggest power of two; 1024 is usually a good choice).
    //   nfreq, freq_lo_MHz, freq_hi_MHz = specification of frequency channels (assumed equally spaced).
    //   dm = dispersion measure in its standard units (pc cm^{-3}).
    //   sm = scattering measure, which we define to be scattering time in milliseconds (not seconds!) at 1 GHz.
    //   intrinsic_width = frequency-independent Gaussian width in seconds (not milliseconds).
    //   fluence = integrated flux in units Jy-s (where "Jy" really means "whatever units the output of add_to_timestream() has")
    //   undispersed_arrival_time = arrival time of pulse as freq->infty, in seconds relative to an arbitrary origin
    //
    single_pulse(int pulse_nt, int nfreq, double freq_lo_MHz, double freq_hi_MHz, 
		 double dm, double sm, double intrinsic_width, double fluence, 
		 double spectral_index, double undispersed_arrival_time);

    void set_fluence(double fluence);
    void set_spectral_index(double spectral_index);
    void set_undispersed_arrival_time(double undispersed_arrival_time);
    void get_endpoints(double &t0, double &t1) const;

    //
    // This routine adds the pulse to a "block" of (frequency, time) samples.
    // It is sometimes called incrementally, as a stream of blocks generated.
    //
    // The 'out_t0' and 'out_t1' args are the endpoints of the sampled region, in seconds relative 
    // to the undispersed_arrival_time.
    //
    // The 'out' arg is a 2d array with shape (nfreq, out_nt).
    // The 'stride' is the pointer offset between frequencies.  If stride=0 then it defaults to out_nt.
    //
    template<typename T> void add_to_timestream(T *out, double out_t0, double out_t1, int out_nt, int stride=0) const;
    
    //
    // Returns total signal-to-noise for all frequency channels and time samples combined.
    // The signal-to-noise of a sampled pulse depends on 'sample_dt', the length of a sample in seconds.
    //
    // In principle, it also depends on 'sample_t0', the starting time of an arbitrarily chosen sample,
    // although this dependence will be weak in realistic cases!
    //
    double get_signal_to_noise(double sample_dt, double sample_t0=0.0, double sample_rms=1.0) const;

    // String representation
    void print(std::ostream &os) const;
    std::string str() const;

    // Internal helper function
    void _compute_freq_wt();

    // We make the single_pulse noncopyable, even though the default copy constructor is a sensible "deep" copy.  
    // We do this to catch performance bugs, since a deep copy of a single_pulse is probably unintentional.
    single_pulse(const single_pulse &) = delete;
    single_pulse& operator=(const single_pulse &) = delete;
};


inline std::ostream &operator<<(std::ostream &os, const single_pulse &sp) { sp.print(os); return os; }


}  // namespace simpulse

#endif // _SIMPULSE_HPP
