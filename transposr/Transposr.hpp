#pragma once
#include <stdexcept>
#include <jack/jack.h>
#include <jack/midiport.h>

using namespace std;

class Transposr
{
private:
  jack_client_t *client;
  jack_port_t *midi_in, *midi_out;
  int8_t offset;

public:
  Transposr(const char client_name[], int8_t o) : offset{o}
  {
    client = jack_client_open(client_name, static_cast<jack_options_t>(0), NULL);
	  if (client == 0)
		  throw runtime_error{"jack server not running?"};

	  midi_in = jack_port_register (client, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

    string name = (offset >= 0 ? "+" : "") + to_string(static_cast<int>(offset));
    midi_out = jack_port_register(client, name.c_str(), JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
  }

  void start()
  {
	  jack_set_process_callback (client, process, this);
	  if ( jack_activate(client) )
		  throw runtime_error{"cannot activate client"};
  }

  ~Transposr()
  {
  	jack_client_close(client);
  }

  static int process(jack_nframes_t frames, void *arg)
  {
    Transposr *transposr = static_cast<Transposr *>(arg);
	  // handle midi
	  void *port_buffer_in = jack_port_get_buffer(transposr->midi_in, frames);
	  void *port_buffer_out = jack_port_get_buffer(transposr->midi_out, frames);
    jack_midi_clear_buffer(port_buffer_out);

	  jack_midi_event_t event;
    for ( int event_index = 0;
          !jack_midi_event_get (&event, port_buffer_in, event_index);
          event_index ++ )
	  {
      switch ( event.buffer[0] & 0xF0 )
      {
        case 0x90:
        case 0x80:
          event.buffer[1] += transposr->offset;
          jack_midi_event_write(port_buffer_out, event.time, event.buffer, event.size);
          break;
      }
	  }

	  return 0;
  }
};

