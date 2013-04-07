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

## Differences from JavaScript

There are a few major differences in the DOM API for Ruby.

- Methods and attributes are specified in `underscore_case` instead of `camelCase`.
- The `window` variable is accessible from only the top-most scope. Elsewhere you can use the global `$window`.
- Ruby accepts a `Proc` or a block for anonymous functions. For example:

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

## Warning
This is a work in progress. Please report any issues you find.
