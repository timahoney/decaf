# Decaf

**Decaf** is a modification of WebKit that runs Ruby in the browser. You can access the DOM, use the inspector, and do most other things that JavaScript can do. For now, it only builds on the Mac. You can [download a binary](http://trydecaf.org/latest?utm_source=github) or you can [build it yourself](http://www.webkit.org/building/build.html).

```html
<script type='text/ruby'>
    window.onload do
        introduction = document.create_element('p')
        introduction.inner_text = 'Hello, world!'
        document.body.append_child(introduction)
    end
</script>
```

Run a Ruby script on a page using `<script type=‘text/ruby’>`. The DOM API will be in `underscore_case` instead of `camelCase`.

### Callbacks and listeners
The DOM API uses anonymous functions for callbacks and listeners. In Ruby, we use a `Proc` or a block.

```ruby
# Ruby with explicit Procs
$window.set_timeout(Proc.new { console.log('Hello!') }, 1000)
$window.onload = Proc.new do |event|
  console.log('The window loaded.')
end

# Ruby with implicit blocks
$window.set_timeout(1000) { console.log('Hello!') }
$window.onload do |event|
  console.log('The window loaded.')
end
```

### The `window` object

In JavaScript, you can access `window` from mostly anywhere. In Ruby, use the global `$window` object. You can access the plain `window` object from the top-most scope, but everywhere else requires you to use the global.

## Warning
This is a work in progress. Please report any issues you find.
