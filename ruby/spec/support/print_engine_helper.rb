require 'mkfifo'
require 'fileutils'

module PrintEngineHelper

  def self.included(including_class)
    including_class.class_exec do
      let(:command_pipe) { tmp_dir 'command_pipe' }
      let(:print_data_dir) { tmp_dir 'print_data' }
      let(:printer_status_file) { tmp_dir 'printer_status' }

      # Helper methods provided by this module require a temporary directory
      if metadata[:client]
        # Client specs are asynchronous and need the async tmp dir hooks
        metadata[:tmp_dir_async] = true
      else
        # Otherwise just use the synchronous hooks
        metadata[:tmp_dir] = true
      end
    end
  end

  def create_printer_status_file
    FileUtils.touch(printer_status_file)
    Smith::Settings.printer_status_file = printer_status_file
  end

  def set_printer_status(status_values)
    File.write(printer_status_file, printer_status(status_values).to_json + "\n")
  end

  def printer_status(vars)
    { Smith::STATE_PS_KEY => vars[:state], Smith::UISUBSTATE_PS_KEY => vars[:substate] }
  end

  def create_command_pipe
    File.mkfifo(command_pipe)
    Smith::Settings.command_pipe = command_pipe
  end

  def open_command_pipe
    @command_pipe_io = File.open(command_pipe, 'r+')
  end

  def close_command_pipe
    @command_pipe_io.close
  end

  def next_command_in_command_pipe
    Timeout::timeout(0.1) { @command_pipe_io.gets.sub("\n", '') }
  rescue Timeout::Error
    fail 'timeout waiting to read from command pipe, pipe does not contain a line to be read'
  end

  def create_print_data_dir
    FileUtils.mkdir(Smith::Settings.print_data_dir = print_data_dir)
  end

  def keys_to_symbols(hash)
    JSON.parse(hash.to_json, symbolize_names: true)
  end

end
