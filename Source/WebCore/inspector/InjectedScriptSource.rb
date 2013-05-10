# Copyright (C) 2007 Apple Inc.  All rights reserved.
# Copyright (C) 2013 Tim Mahoney (tim.mahoney@me.com)
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
# 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
#     its contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

class InjectedScript

  PRIMTIVE_TYPES = { 
    NilClass    => true,
    Numeric     => true,
    String      => true,
    TrueClass   => true,
    FalseClass  => true
  }

  attr_reader :injected_script_host
  attr_reader :injected_script_id

  def initialize(injected_script_host = nil, inspected_window = nil, injected_script_id = nil)
    @injected_script_host = injected_script_host
    @inspected_window = inspected_window
    @injected_script_host.instance_variable_set(:@inspected_window, @inspected_window)
    @injected_script_id = injected_script_id

    @last_bound_object_id = 1
    @id_to_wrapped_object = {}
    @id_to_object_group_name = {}
    @object_groups = {}
    @modules = {}
  end

  def is_primitive_value(obj)
    # FIXME: Do we need to check for if the object is an HTMLAllCollection like in the JS InjectedScript?
    PRIMTIVE_TYPES.any? { |type| obj.is_a? type[0] }
  end

  def wrap_object(object, group_name, can_access_inspected_window, generate_preview)
    if (can_access_inspected_window)
      return _wrap_object(object, group_name, false, generate_preview)
    end

    result = {}
    result[:type] = object.class.to_s
    if (is_primitive_value(object))
      result[:value] = object;
    else
      result[:description] = object.to_s
    end

    result
  end
  alias_method :wrapObject, :wrap_object

  def _wrap_object(object, object_group_name = nil, force_value_type = false, generate_preview = false)
    result = nil
    begin
      result = RemoteObject.new(self, object, object_group_name, force_value_type, generate_preview)
    rescue => e
      p e.backtrace.join("\n\t")
      begin
        description = _describe($!)
      rescue
        description = "<failed to convert exception to string>";
      end

      result = RemoteObject.new(self, description);
    end

    result
  end

  def inspect_node(object)
    _inspect(object)
  end

  def _inspect(object)
    return nil if !object

    object_id = _wrap_object(object, "")
    hints = {}

    case _describe(object)
    when "Database"
      database_id = @injected_script_host.database_id(object)
      hints[:databaseId] = database_id if database_id
    when "Storage"
      storage_id = @injected_script_host.storage_id(object)
      hints[:domStorageId] = storage_id if storage_id
    end

    @injected_script_host.inspect(object_id, hints)

    object
  end

  def bind(object, object_group_name)
    id = @last_bound_object_id
    @last_bound_object_id += 1
    @id_to_wrapped_object[id] = object;
    object_id = "{\"injectedScriptId\":#{@injected_script_id},\"id\":#{id}}"
    if (object_group_name)
      group = @object_groups[object_group_name]
      if (!group)
        group = []
        @object_groups[object_group_name] = group
      end
      group.push(id)
      @id_to_object_group_name[id] = object_group_name
    end

    object_id
  end

  def _parse_object_id(object_id)
    /injectedScriptId[^\d]+([\d]+)/.match(object_id)
    injected_script_id = $1.to_i
    /id[^\d]+([\d]+)/.match(object_id)
    id = $1.to_i
    { :injected_script_id => injected_script_id, :id => id }
  end

  def release_object_group(object_group_name)
    group = @object_groups[object_group_name]
    return if (!group)

    group.each { |obj| _release_object(obj) }
    @object_groups[object_group_name] = nil
  end
  alias_method :releaseObjectGroup, :release_object_group

  def dispatch(method_name, args)

    # FIXME: Does this actually work? Is this ever called from anywhere?
    args_array = @injected_script_host.evaluate(args)
    method_sym = method_name.to_sym
    if (respond_to? method_sym)
      send(method_name.to_sym, args_array)
    else
      @inspected_window.console.error("Web Inspector error: InjectedScript.#{method_name} does not exist.")
      nil      
    end
  end

  def get_properties(object_id, own_properties)
    parsed_object_id = _parse_object_id(object_id)
    object = _object_for_id(parsed_object_id)
    object_group_name = @id_to_object_group_name[parsed_object_id[:id]]

    return false if !object

    descriptors = _property_descriptors(object, own_properties)

    # Go over properties and wrap object values.
    descriptors.each do |descriptor|
        descriptor[:get] = _wrap_object(descriptor[:get], object_group_name) if descriptor[:get]
        descriptor[:set] = _wrap_object(descriptor[:set], object_group_name) if descriptor[:set]
        descriptor[:value] = _wrap_object(descriptor[:value], object_group_name)
        descriptor[:configurable] = false if !descriptor[:configurable]
        descriptor[:enumerable] = false if !descriptor[:enumerable]
    end

    descriptors
  end
  alias_method :getProperties, :get_properties

  def get_internal_properties(object_id)
    parsed_object_id = _parse_object_id(object_id)
    object = _object_for_id(parsed_object_id)
    object_group_name = @id_to_object_group_name[parsed_object_id[:id]]

    return false if !object

    descriptors = []
    internal_properties = @injected_script_host.get_internal_properties(object)
    if (internal_properties)
      internal_properties.each do |property|
        descriptor = { 
          :name   => property[:name],
          :value  => _wrap_object(property[:value], object_group_name)
        }
        descriptors.push(descriptor)
      end
    end

    descriptors
  end
  alias_method :getInternalProperties, :get_internal_properties

  def get_function_details(function_id)
    parsed_function_id = _parse_object_id(function_id)
    func = _object_for_id(parsed_function_id)
    if (!func.is_a? Proc)
      return "Cannot resolve function by id."
    end

    details = @injected_script_host.function_details(func)
    if (details[:rawScopes])
      object_group_name = @id_to_object_group_name[parsed_function_id[:id]]
      raw_scopes = details[:rawScopes]
      scopes = []
      details[:rawScopes] = nil
      # raw_scopes.each { |scope| scopes.push(CallFrameProxy._create_scope_json(scope[:type], scope[:object], object_group_name, self)) }
      details[:scopeChain] = scopes;
    end

    details
  end
  alias_method :getFunctionDetails, :get_function_details

  def release_object(object_id)
    parsed_object_id = _parse_object_id(object_id)
    _release_object(parsed_object_id[:id])
  end
  alias_method :releaseObject, :release_object

  def _release_object(id)
    @id_to_wrapped_object[id] = nil
    @id_to_object_group_name[id] = nil
  end

  def get_primitive_type_completions(type)
    case type
    when "string"
      return _get_completions("string")
    when "boolean"
      return _get_completions(true)
    when "number"
      return _get_completions(33.33)
    end

    return _get_completions(nil)
  end
  alias_method :getPrimitiveTypeCompletions, :get_primitive_type_completions

  def get_completions(object_id)
    object = find_object_by_id(object_id)
    return _get_completions(object)
  end
  alias_method :getCompletions, :get_completions

  def _get_completions(object)
    return object.methods
  end

  def _property_descriptors(object, own_properties)

    if (object.is_a? Numeric)
      return [{
          :name         => object.to_s,
          :value        => object,
          :writable     => true,
          :configurable => true,
          :enumerable   => true
      }]
    end

    if (object.is_a? Hash)
      return object.map do |key, value|
        {
          :name         => key.to_s,
          :value        => value,
          :writable     => true,
          :configurable => true,
          :enumerable   => true
        }
      end
    end

    subtype = _subtype(object)
    if (subtype == "array")
      descriptors = []
      object.each_with_index do |item, i|
        descriptors.push({
          :name         => i.to_s,
          :value        => item,
          :writable     => true,
          :configurable => true,
          :enumerable   => true
        })
      end

      return descriptors
    end

    descriptors = []
    if (object.class.respond_to? :dom_binding_attributes)
      dom_attributes = object.class.dom_binding_attributes.map do |name|
          descriptor = { :name         => name,
                         :writable     => object.respond_to?("#{name.to_s}=".to_sym),
                         :configurable => false,
                         :enumerable   => true }
        begin
          descriptor[:value] = object.method(name).call
        rescue Exception => e
          p e.backtrace
          descriptor[:value] = e
          descriptor[:wasThrown] = true
        end

        descriptor
      end
      descriptors.concat(dom_attributes)
    end

    ivar_names = object.instance_variables.select { |name| name.to_s != '@dom_binding_attributes' }
    ivars = ivar_names.map do |name|
      {
        :name         => name,
        :value        => object.instance_variable_get(name),
        :writable     => object.respond_to?("#{name.to_s}=".to_sym),
        :configurable => false,
        :enumerable   => true
      }
    end

    if (object.is_a? Binding)
      locals = object.eval('local_variables').map do |name|
        {
          :name         => name,
          :value        => object.eval(name.to_s),
          :writable     => true,
          :configurable => false,
          :enumerable   => true
        }
      end
      descriptors.concat(locals)
    end

    descriptors.concat(ivars)
    return descriptors
  end

  def evaluate(expression, object_group, inject_command_line_api, return_by_value, generate_preview)
    return _evaluate_and_wrap(@injected_script_host.method(:evaluate), @injected_script_host, expression, object_group, false, inject_command_line_api, return_by_value, generate_preview)
  end

  def call_function_on(object_id, expression, args, return_by_value)
    parsed_object_id = _parse_object_id(object_id)
    object = _object_for_id(parsed_object_id)
    return "Could not find object with given id" if !object

    if (args)
      resolved_args = []
      args = @injected_script_host.evaluate(args)
      args.each do |arg|
        object_id = arg[:object_id]
        if (object_id)
          parsed_arg_id = _parse_object_id(object_id)
          if (!parsed_arg_id || parsed_arg_id[:injected_script_id] != @injected_script_id)
            return "Arguments should belong to the same world as the target object."
          end

          resolved_arg = _object_for_id(parsed_arg_id)
          if (!resolved_arg)
            return "Could not find object with given id" 
          end

          resolved_args.push(resolved_arg)
        elsif (arg[:value])
          resolved_args.push(arg[:value])
        else
          resolved_args.push(nil)
        end
      end
    end

    begin
      object_group = @id_to_object_group_name[parsed_object_id[:id]]
      func = @injected_script_host.evaluate(expression)
      if (!func is_a? Method)
        return "Given expression does not evaluate to a function"
      end

      result = func.call(resolved_args)
      return { :wasThrown => false, :result => _wrap_object(result, object_group, return_by_value) }
    rescue Exception => e
      p e.backtrace
      return _create_thrown_value($!, object_group)
    end
  end
  alias_method :callFunctionOn, :call_function_on

  def _evaluate_and_wrap(eval_function, object, expression, object_group, is_eval_on_call_frame, inject_command_line_api, return_by_value, generate_preview)
    begin
      result = _evaluate_on(eval_function, object, object_group, expression, is_eval_on_call_frame, inject_command_line_api)
      return { 
        :wasThrown => false, 
        :result    => _wrap_object(result, object_group, return_by_value, generate_preview) 
      }
    rescue Exception => e
      p e.backtrace
      return _create_thrown_value(e, object_group)
    end
  end

  def _create_thrown_value(value, object_group)
    remote_object = _wrap_object(value, object_group)
    remote_object.description = "#{value.class}: #{value.message}"

    return { :wasThrown => true, :result => remote_object }
  end

  def _evaluate_on(eval_function, object, object_group, expression, is_eval_on_call_frame, inject_command_line_api)
    if (inject_command_line_api && @inspected_window.console)
      # FIXME: Implement CommandLineAPI
    end

    result = eval_function.call(expression)
    if (object_group == "console")
      @last_result = result
    end
    return result
  end

  def wrap_call_frames(call_frame)
    return false if !call_frame

    result = []
    depth = 0
    while (call_frame)
      result.push(CallFrameProxy.new(depth, call_frame, self))
      depth += 1
      call_frame = call_frame.caller
    end
    return result
  end
  alias_method :wrapCallFrames, :wrap_call_frames

  def evaluate_on_call_frame(top_call_frame, call_frame_id, expression, object_group, inject_command_line_api, return_by_value, generate_preview)
    call_frame = _call_frame_for_id(top_call_frame, call_frame_id)
    if (!call_frame)
      return "Could not find call frame with given id"
    end

    return _evaluate_and_wrap(call_frame.method(:evaluate), call_frame, expression, object_group, true, inject_command_line_api, return_by_value, generate_preview)
  end
  alias_method :evaluateOnCallFrame, :evaluate_on_call_frame

  def restart_frame(top_call_frame, call_frame_id)
    call_frame = _call_frame_for_id(top_call_frame, call_frame_id)
    if (!call_frame)
      return "Could not find call frame with given id"
    end
    result = call_frame.restart()
    if (!result)
      result = "Restart frame is not supported"
    end
    return result
  end
  alias_method :restartFrame, :restart_frame

  def _parse_call_frame_id(call_frame_id)
    /ordinal[^\d]+([\d]+)/.match(call_frame_id)
    ordinal = $1.to_i
    /injectedScriptId[^\d]+([\d]+)/.match(call_frame_id)
    injected_script_id = $1.to_i
    { :injected_script_id => injected_script_id, :ordinal => ordinal }
  end

  def _call_frame_for_id(top_call_frame, call_frame_id)
    # FIXME: This doesn't seem right at all. It was copied from InjectedScriptSource.js
    parsed_call_frame_id = _parse_call_frame_id(call_frame_id)
    ordinal = parsed_call_frame_id[:ordinal]
    call_frame = top_call_frame
    ordinal -= 1
    while (ordinal >= 0 && call_frame)
      call_frame = call_frame.caller
    end
    return call_frame
  end

  def _object_for_id(object_id)
    @id_to_wrapped_object[object_id[:id]]
  end

  def find_object_by_id(object_id)
    parsed_object_id = _parse_object_id(object_id)
    return _object_for_id(parsed_object_id)
  end
  alias_method :findObjectById, :find_object_by_id

  def node_for_object_id(object_id)
    object = find_object_by_id(object_id)
    if (!object || _subtype(object) != "node")
      return nil
    end
    return object
  end
  alias_method :nodeForObjectId, :node_for_object_id

  def module(name)
    return @modules[name]
  end

  def inject_module(name, source)
    # FIXME: This may not work. The behavior is different in Ruby than in JavaScript.
    # The source should be the class definition, then we'll instantiate an object from it.
    @modules[name] = nil
    @injected_script_host.evaluate(source)
    module_class = Object.const_get(name)
    instance = module_class.new(@injected_script_host, @inspected_window, @injected_script_id)
    @modules[name] = instance
    return instance
  end
  alias_method :injectModule, :inject_module

  def _subtype(obj)
    return "null" if obj.nil?
    return "null" if is_primitive_value(obj)

    precise_type = @injected_script_host.type(obj)
    return precise_type if (precise_type)

    return nil
  end

  def _describe(obj)
    return nil if is_primitive_value(obj)

    return ":#{obj.to_s}" if obj.is_a? Symbol
    if (obj.is_a? Module)
      return obj.name if obj.name
      return obj.to_s
    end

    subtype = _subtype(obj)
    return obj.to_s if subtype == "regexp"
    return obj.to_s if subtype == "date"

    if (subtype == "node")
      description = obj.node_name.downcase
      case obj.node_type
      when 1  # Node.ELEMENT_NODE
        description = "<#{description}>"
      when 10 # Node.DOCUMENT_TYPE_NODE
        description = "<!DOCTYPE #{description}>"
      end
      return description
    end

    if (obj.is_a? Proc or obj.is_a? Method)
      # FIXME: Put the source location here.
      name = "function"
      name = obj.name if obj.respond_to? :name
      parameter_strings = obj.parameters.map do |pair| 
        string = pair[1].to_s 
        string += " [optional]" if pair[0] == :opt
      end
      parameters = parameter_strings.join(', ')
      return "#{name}(#{parameters})"
    end

    class_name = @injected_script_host.internal_constructor_name(obj)
    if (subtype == "array" and obj.respond_to?(:length))
      class_name += "[#{obj.length}]"
    end

    return class_name
  end

  def typeof(object)
    return "number" if object.is_a? Numeric or object.is_a? Symbol
    return "string" if object.is_a? String
    return "boolean" if object.is_a? TrueClass or object.is_a? FalseClass
    return "function" if object.is_a? Proc or object.is_a? Method
    return "object"
  end

  def build_array_fragment(object_id, from_index, to_index)
    object = find_object_by_id(object_id)
    result = {}
    (from_index..to_index).each do |i|
      result[i] = object[i]
    end

    _wrap_object(result);
  end
  alias_method :buildArrayFragment, :build_array_fragment

  def build_object_fragment(object_id)
    
    # This is only used for arrays. Just give the length.
    object = find_object_by_id(object_id)
    result = {
      :length => object.length
    }

    _wrap_object(result)
  end
  alias_method :buildObjectFragment, :build_object_fragment

  class RemoteObject
    attr_accessor :description

    def initialize(injected_script, object, object_group_name = nil, force_value_type = false, generate_preview = false)
      @type = injected_script.typeof(object)
      if (injected_script.is_primitive_value(object) || object == nil || force_value_type)
        @value = object
        @subtype = "null" if object.nil?
        @description = object.to_s if @type == "number"
        return
      end

      if (object.is_a? Symbol)
        @value = object.object_id
        @description = object.inspect
        return
      end

      @objectId = injected_script.bind(object, object_group_name)
      @subtype = injected_script._subtype(object) if injected_script._subtype(object)
      @className = injected_script.injected_script_host.internal_constructor_name(object)
      @description = injected_script._describe(object)

      if (generate_preview and @type == "object")
        _generate_preview(object, injected_script)
      end
    end

    def _generate_preview(object, injected_script)
      @preview = {}
      @preview[:lossless] = true
      @preview[:overflow] = false
      @preview[:properties] = []

      elements_to_dump = (@subtype == "array") ? 100 : 5
      _generate_proto_preview(object, elements_to_dump, injected_script)
    end

    def _generate_descriptor(name, value, injected_script)
      max_length = 100
      type = injected_script.typeof(value)
      descriptor = { :name => name.to_s, :type => type }
      if (value.nil?)
        descriptor[:subtype] = "null"

      elsif (injected_script.is_primitive_value(value))
        if (type == "string")
          value = _abbreviate_string(value, max_length, true)
          value = "#{value.gsub(/\n/, "\u21B5")}"
        end
        descriptor[:value] = value.to_s

      else
        @preview[:lossless] = false
        subtype = injected_script._subtype(value)
        description = ""
        if (type != "function")
          description = _abbreviate_string(injected_script._describe(value), max_length, subtype == "regexp")
        end

        descriptor[:value] = description
        descriptor[:subtype] = subtype if (subtype)
      end

      descriptor
    end

    def _generate_hash_preview(object, elements_to_dump, injected_script)
      object.each_pair do |key, value|
        if (@preview[:properties].length >= elements_to_dump)
          @preview[:overflow] = true
          @preview[:lossless] = false
          return
        end

        descriptor = _generate_descriptor(key, value, injected_script)
        @preview[:properties].push(descriptor)
      end
    end

    def _generate_array_preview(object, elements_to_dump, injected_script)
      if (elements_to_dump <= object.size)
        @preview[:overflow] = true
        @preview[:lossless] = false
      end

      elements_to_dump = [object.size, elements_to_dump].min
      object[0..elements_to_dump].each_with_index do |value, i|
        name = i.to_s
        property = _generate_descriptor(name, value, injected_script)
        @preview[:properties].push(property)
      end
    end

    def _generate_proto_preview(object, elements_to_dump, injected_script)

      if (object.is_a? Hash)
        _generate_hash_preview(object, elements_to_dump, injected_script)
        return
      end

      if (object.is_a? Array)
        _generate_array_preview(object, elements_to_dump, injected_script)
        return
      end

      for i in (0..object.instance_variables.length - 1)
        if (@preview[:properties].length >= elements_to_dump)
          @preview[:overflow] = true
          @preview[:lossless] = false
          return
        end

        name = object.instance_variables[i]
        next if (name.to_s == "@dom_binding_attributes")
        next if (@subtype == "array" && name.to_s == "@length")

        value = object.instance_variable_get(name)
        property = _generate_descriptor(name, value, injected_script)
        @preview[:properties].push(property)
      end

      if (object.class.respond_to? :dom_binding_attributes)
        object.class.dom_binding_attributes.each do |name|
          if (@preview[:properties].length >= elements_to_dump)
            @preview[:overflow] = true
            @preview[:lossless] = false
            break
          end

          begin
            value = object.method(name).call
            property = _generate_descriptor(name, value, injected_script)
            @preview[:properties].push(property)
          rescue Exception => e
            p e.backtrace
            next
          end
        end
      end
    end

    def _abbreviate_string(string, max_length, middle)
      return string if (string.length <= max_length)
      if (middle)
        left_half = max_length / 2
        right_half = max_length - left_half - 1
        return string[0..left_half] + "\u2026" + string[string.length - right_half..right_half]
      end

      return string[0..max_length] + "\u2026"
    end
  end

  class CallFrameProxy

    def initialize(ordinal, call_frame, injected_script)
      @callFrameId = "{\"ordinal\":#{ordinal},\"injectedScriptId\":#{injected_script.injected_script_id}}"
      @functionName = call_frame.type == "function" ? call_frame.function_name : ""
      @location = { :scriptId => call_frame.source_id.to_s, :lineNumber => call_frame.line }
      @scopeChain = _wrap_scope_chain(call_frame, injected_script)
      @this = injected_script._wrap_object(call_frame.this_object, "backtrace")
    end

    SCOPE_TYPE_NAME_STRINGS = ['global', 'local', 'with', 'closure', 'catch']

    def _create_scope_json(scope_type_code, scope_object, group_id, injected_script)
      json = {}
      json[:type] = SCOPE_TYPE_NAME_STRINGS[scope_type_code]
      json[:object] = injected_script._wrap_object(scope_object, group_id)
      json
    end

    def _wrap_scope_chain(call_frame, injected_script)
      scope_chain = call_frame.scope_chain
      scope_chain_proxy = []
      scope_chain.each_with_index do |scope, i|
        scope_json = _create_scope_json(call_frame.scope_type(i), scope, "backtrace", injected_script)
        scope_chain_proxy.push(scope_json)
      end

      scope_chain_proxy
    end
  end
end

# FIXME: Implement CommandLineAPI

# /**
#  * @constructor
#  * @param {CommandLineAPIImpl} commandLineAPIImpl
#  * @param {Object} callFrame
#  */
# function CommandLineAPI(commandLineAPIImpl, callFrame)
# {
#     /**
#      * @param {string} member
#      * @return {boolean}
#      */
#     function inScopeVariables(member)
#     {
#         if (!callFrame)
#             return false;

#         var scopeChain = callFrame.scopeChain;
#         for (var i = 0; i < scopeChain.length; ++i) {
#             if (member in scopeChain[i])
#                 return true;
#         }
#         return false;
#     }

#     for (var i = 0; i < CommandLineAPI.members_.length; ++i) {
#         var member = CommandLineAPI.members_[i];
#         if (member in inspectedWindow || inScopeVariables(member))
#             continue;

#         this[member] = bind(commandLineAPIImpl[member], commandLineAPIImpl);
#     }

#     for (var i = 0; i < 5; ++i) {
#         var member = "$" + i;
#         if (member in inspectedWindow || inScopeVariables(member))
#             continue;

#         this.__defineGetter__("$" + i, bind(commandLineAPIImpl._inspectedObject, commandLineAPIImpl, i));
#     }

#     this.$_ = injectedScript._lastResult;
# }

# /**
#  * @type {Array.<string>}
#  * @const
#  */
# CommandLineAPI.members_ = [
#     "$", "$$", "$x", "dir", "dirxml", "keys", "values", "profile", "profileEnd",
#     "monitorEvents", "unmonitorEvents", "inspect", "copy", "clear", "getEventListeners"
# ];

# /**
#  * @constructor
#  */
# function CommandLineAPIImpl()
# {
# }

# CommandLineAPIImpl.prototype = {
#     /**
#      * @param {string} selector
#      * @param {Node=} start
#      */
#     $: function (selector, start)
#     {
#         if (this._canQuerySelectorOnNode(start))
#             return start.querySelector(selector);

#         var result = document.querySelector(selector);
#         if (result)
#             return result;
#         if (selector && selector[0] !== "#") {
#             result = document.getElementById(selector);
#             if (result) {
#                 console.warn("The console function $() has changed from $=getElementById(id) to $=querySelector(selector). You might try $(\"#%s\")", selector );
#                 return null;
#             }
#         }
#         return result;
#     },

#     /**
#      * @param {string} selector
#      * @param {Node=} start
#      */
#     $$: function (selector, start)
#     {
#         if (this._canQuerySelectorOnNode(start))
#             return start.querySelectorAll(selector);
#         return document.querySelectorAll(selector);
#     },

#     /**
#      * @param {Node|undefined} node
#      * @return {boolean}
#      */
#     _canQuerySelectorOnNode: function(node)
#     {
#         return !!node && InjectedScriptHost.type(node) === "node" && (node.nodeType === Node.ELEMENT_NODE || node.nodeType === Node.DOCUMENT_NODE || node.nodeType === Node.DOCUMENT_FRAGMENT_NODE);
#     },

#     /**
#      * @param {string} xpath
#      * @param {Node=} context
#      */
#     $x: function(xpath, context)
#     {
#         var doc = (context && context.ownerDocument) || inspectedWindow.document;
#         var result = doc.evaluate(xpath, context || doc, null, XPathResult.ANY_TYPE, null);
#         switch (result.resultType) {
#         case XPathResult.NUMBER_TYPE:
#             return result.numberValue;
#         case XPathResult.STRING_TYPE:
#             return result.stringValue;
#         case XPathResult.BOOLEAN_TYPE:
#             return result.booleanValue;
#         default:
#             var nodes = [];
#             var node;
#             while (node = result.iterateNext())
#                 nodes.push(node);
#             return nodes;
#         }
#     },

#     dir: function()
#     {
#         return console.dir.apply(console, arguments)
#     },

#     dirxml: function()
#     {
#         return console.dirxml.apply(console, arguments)
#     },

#     keys: function(object)
#     {
#         return Object.keys(object);
#     },

#     values: function(object)
#     {
#         var result = [];
#         for (var key in object)
#             result.push(object[key]);
#         return result;
#     },

#     profile: function()
#     {
#         return console.profile.apply(console, arguments)
#     },

#     profileEnd: function()
#     {
#         return console.profileEnd.apply(console, arguments)
#     },

#     /**
#      * @param {Object} object
#      * @param {Array.<string>|string=} types
#      */
#     monitorEvents: function(object, types)
#     {
#         if (!object || !object.addEventListener || !object.removeEventListener)
#             return;
#         types = this._normalizeEventTypes(types);
#         for (var i = 0; i < types.length; ++i) {
#             object.removeEventListener(types[i], this._logEvent, false);
#             object.addEventListener(types[i], this._logEvent, false);
#         }
#     },

#     /**
#      * @param {Object} object
#      * @param {Array.<string>|string=} types
#      */
#     unmonitorEvents: function(object, types)
#     {
#         if (!object || !object.addEventListener || !object.removeEventListener)
#             return;
#         types = this._normalizeEventTypes(types);
#         for (var i = 0; i < types.length; ++i)
#             object.removeEventListener(types[i], this._logEvent, false);
#     },

#     /**
#      * @param {*} object
#      * @return {*}
#      */
#     inspect: function(object)
#     {
#         return injectedScript._inspect(object);
#     },

#     copy: function(object)
#     {
#         if (injectedScript._subtype(object) === "node")
#             object = object.outerHTML;
#         InjectedScriptHost.copyText(object);
#     },

#     clear: function()
#     {
#         InjectedScriptHost.clearConsoleMessages();
#     },

#     /**
#      * @param {Node} node
#      */
#     getEventListeners: function(node)
#     {
#         return InjectedScriptHost.getEventListeners(node);
#     },

#     /**
#      * @param {number} num
#      */
#     _inspectedObject: function(num)
#     {
#         return InjectedScriptHost.inspectedObject(num);
#     },

#     /**
#      * @param {Array.<string>|string=} types
#      * @return {Array.<string>}
#      */
#     _normalizeEventTypes: function(types)
#     {
#         if (typeof types === "undefined")
#             types = [ "mouse", "key", "touch", "control", "load", "unload", "abort", "error", "select", "change", "submit", "reset", "focus", "blur", "resize", "scroll", "search", "devicemotion", "deviceorientation" ];
#         else if (typeof types === "string")
#             types = [ types ];

#         var result = [];
#         for (var i = 0; i < types.length; i++) {
#             if (types[i] === "mouse")
#                 result.splice(0, 0, "mousedown", "mouseup", "click", "dblclick", "mousemove", "mouseover", "mouseout", "mousewheel");
#             else if (types[i] === "key")
#                 result.splice(0, 0, "keydown", "keyup", "keypress", "textInput");
#             else if (types[i] === "touch")
#                 result.splice(0, 0, "touchstart", "touchmove", "touchend", "touchcancel");
#             else if (types[i] === "control")
#                 result.splice(0, 0, "resize", "scroll", "zoom", "focus", "blur", "select", "change", "submit", "reset");
#             else
#                 result.push(types[i]);
#         }
#         return result;
#     },

#     /**
#      * @param {Event} event
#      */
#     _logEvent: function(event)
#     {
#         console.log(event.type, event);
#     }
# }

# injectedScript._commandLineAPIImpl = new CommandLineAPIImpl();
# return injectedScript;
# })
InjectedScript
