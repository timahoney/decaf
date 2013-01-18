# Decaf
This project allows you to use Ruby to access the DOM. It's a modification of the WebKit browser engine. For now, you need to [build it yourself](http://www.webkit.org/building/build.html). It only builds on the Mac at the moment.

```ruby
<script type='text/ruby'>
  window.onload do
    introduction = document.create_element('p')
    introduction.inner_text = 'Hello, world!'
    document.body.append_child(introduction)
  end
</script>
```

## Using Ruby in the browser

You include a Ruby script on a page using `<script type=‘text/ruby’>`. You can access the standard DOM API from Ruby, but it will be in `underscore_case` instead of `camelCase`.
  
### Anonymous functions
The DOM API uses a lot of callbacks, listeners, and other types of anonymous functions. The equivalent in Ruby is a `Proc` or a block.

```javascript
// JavaScript
window.setTimeout(function() {
  console.log('Hello!');
}, 1000);

window.onload = function(event) {
  console.log('The window loaded.');
}
```

```ruby
# Ruby with explicit Procs
window.set_timeout(Proc.new { console.log('Hello!') }, 1000)
window.onload = Proc.new do |event|
  console.log('The window loaded.')
end

# Ruby with implicit blocks
window.set_timeout(1000) { console.log('Hello!') }
window.onload do |event|
  console.log('The window loaded.')
end
```

### The `window` object

In JavaScript, you can access `window` from mostly anywhere. In Ruby, you can access `window` from the topmost scope, but you need to use the global `$window` everywhere else.

### The inspector

You can use the Web Inspector by enabling it under _Develop > Enable Web Inspector_. You can use the console and debug Ruby, but profiling doesn't work.

## Warning
This is a work in progress. Please report any issues you find.
