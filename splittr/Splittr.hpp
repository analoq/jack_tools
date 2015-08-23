#pragma once
#include <stdexcept>
#include <vector>
#include <set>
#include <jack/jack.h>
#include <jack/midiport.h>

using namespace std;

class Splittr
{
private:
  jack_client_t *client;
  jack_port_t *midi_in;

  struct Output
  {
    uint8_t key_boundary;
    jack_port_t *midi_out;

    Output(uint8_t k, jack_port_t* m)
      : key_boundary{k}, midi_out{m}
    {
    }

    bool operator<(const Output &other) const
    {
      return key_boundary < other.key_boundary;
    }
  };

  set<Output> outputs;

public:
  Splittr(const char client_name[], set<uint8_t> boundary_keys)
  {
    client = jack_client_open(client_name, static_cast<jack_options_t>(0), NULL);
	  if (client == 0)
		  throw runtime_error{"jack server not running?"};

	  midi_in = jack_port_register (client, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
	  
    outputs.emplace(0,
                    jack_port_register(client, "0", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0));
    int i = 0;
    for ( auto it = boundary_keys.begin();
          it != boundary_keys.end();
          i ++, it++ )
    {
      string name = to_string(*it);
      jack_port_t *midi_out = jack_port_register(client,
                                                 name.c_str(),
                                                 JACK_DEFAULT_MIDI_TYPE,
                                                 JackPortIsOutput,
                                                 0);
	    outputs.emplace(*it, midi_out);
    }
  }

  void start()
  {
	  jack_set_process_callback (client, process, this);
	  if ( jack_activate(client) )
		  throw runtime_error{"cannot activate client"};
  }

  ~Splittr()
  {
  	jack_client_close(client);
  }

  static int process(jack_nframes_t frames, void *arg)
  {
    Splittr *splittr = static_cast<Splittr *>(arg);
	  // handle midi
	  void *port_buffer_in = jack_port_get_buffer(splittr->midi_in, frames);

    for ( const Output &output : splittr->outputs )
    {
  	  void *port_buffer_out = jack_port_get_buffer(output.midi_out, frames);
      jack_midi_clear_buffer(port_buffer_out);
    }

	  jack_midi_event_t event;
    for ( int event_index = 0;
          !jack_midi_event_get (&event, port_buffer_in, event_index);
          event_index ++ )
	  {
      switch ( event.buffer[0] & 0xF0 )
      {
        case 0x90:
        case 0x80:
          auto it = splittr->outputs.upper_bound(Output { event.buffer[1], NULL });
          const Output &output = *--it;
      	  void *port_buffer_out = jack_port_get_buffer(output.midi_out, frames);
          jack_midi_event_write(port_buffer_out, event.time, event.buffer, event.size);
          break;
      }
	  }

	  return 0;
  }
};

