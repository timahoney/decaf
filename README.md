# Decaf

**Decaf** is a modification of WebKit that runs Ruby in the browser. Use Ruby to access the DOM, work with the web standards, and even print to the inspector. Now, you can build an entire web application in Ruby instead of JavaScript.

To use Ruby in the browser, either [download a binary](http://trydecaf.org) or [build it yourself](http://www.webkit.org/building/build.html).

```html
<script type='text/ruby'>
    window.onload do
        introduction = document.create_element('p')
        introduction.inner_text = 'Hello, world!'
        document.body.append_child(introduction)
    end
</script>
```

## Using Ruby in the Browser

The same web platform APIs used by JavaScript are available in Ruby. If you're not familiar with JavaScript and the DOM, then check out these resources:

- __Mozilla's [DOM Reference](https://developer.mozilla.org/en-US/docs/DOM) and [Gecko DOM Reference](https://developer.mozilla.org/en-US/docs/Gecko_DOM_Reference)__
- __[W3C DOM Reference](http://www.w3.org/TR/domcore)__
- __[Web Platform Documentation](http://www.webplatform.org)__

### Differences from JavaScript

There are a few major differences between the APIs in JavaScript and Ruby.

- In Ruby, methods and attributes are specified in `underscore_case` instead of `camelCase`.
- The `window` variable is accessible from only the top-most scope. Elsewhere you can use the global `$window`.
- Ruby accepts `Procs` and blocks as callbacks and listeners. For example:

```ruby
# Ruby with implicit blocks
$window.set_timeout(1000) { console.log('Hello!') }
$window.onload do |event|
  console.log('The window loaded.')
end

# Ruby with explicit Procs
$window.set_timeout(Proc.new { console.log('Hello!') }, 1000)
$window.onload = Proc.new do |event|
  console.log('The window loaded.')
end
```

## Final Notes

If you find any problems, then please report them in the [issues section](https://github.com/timahoney/decaf/issues).



