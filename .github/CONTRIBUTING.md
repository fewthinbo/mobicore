# 🤝 Contributing to mobicore

Thank you for your interest in mobicore! This project is a **client-side integration library** that connects MT servers to our bridge service.

## 🎯 How You Can Contribute

### 📚 **Documentation & Examples**
- **Fix typos** and improve clarity
- **Add installation examples** for different MT setups
- **Create configuration guides** for specific scenarios
- **Translate documentation** to other languages
- **Share integration experiences**

### 🐛 **Bug Reports**
- **Installation issues** on different systems
- **Configuration problems** with MT integration
- **Compilation errors** with various compilers
- **Documentation gaps** or unclear instructions

### 💡 **Community Support**
- **Answer questions** in Discussions
- **Help troubleshoot** installation issues
- **Share successful** implementation stories
- **Provide feedback** on features

## 🚫 What We DON'T Accept

### ❌ **Core Feature Development**
- Bridge server functionality changes
- Network protocol modifications  
- Core business logic alterations
- New packet types or handlers

**Why?** These require our bridge server for testing and integration.

## 📋 How to Contribute

### 🔧 **For Documentation**
1. **Fork** the repository
2. **Edit** documentation files
3. **Test** your changes locally
4. **Submit** a pull request

### 🐛 **For Bug Reports**
1. **Check** existing issues first
2. **Use** our bug report template
3. **Provide** detailed environment info
4. **Include** error logs/screenshots

### 💬 **For Community Support**
1. **Join** our [Discussions](https://github.com/fewthinbo/mobicore/discussions)
2. **Browse** Q&A section for common questions
3. **Share** your implementation experience
4. **Help** other community members

## 📝 Documentation Style

### ✅ **Good Examples**
```bash
# Clear, step-by-step instructions
cd /usr/local/mt-server
git clone https://github.com/fewthinbo/mobicore.git
cmake --preset freebsd-remote-debug
```

### ❌ **Avoid This**
```bash
# Vague instructions
# Just compile and it works
```

## 🧪 Testing Limitations

**Important:** This library requires our bridge server for full functionality testing.

### ✅ **What You Can Test**
- Compilation process
- Installation steps
- Configuration file loading
- Basic integration (without network)

### ❌ **What Requires License**
- Full network communication
- Real-time messaging
- Bridge server connectivity
- Complete feature testing

## 📞 Getting Help

### 🎯 **For Contributors**
- 💬 [Discussions](https://github.com/fewthinbo/mobicore/discussions) - Community help
- 📧 [Email](mailto:mobicore.io@gmail.com) - Direct support
- 🐛 [Issues](https://github.com/fewthinbo/mobicore/issues) - Bug reports

### 📚 **Resources**
- 📖 [Documentation](../README/en.md)
- 🔧 [Installation Guide](../README/docs/this_en.md)
- ⚙️ [Configuration Guide](../README/docs/tech_en.md)

## 📄 License

By contributing documentation or bug reports, you agree that your contributions will be licensed under the [GNU GPL-3.0 License](LICENSE).

---

*Thank you for helping make mobicore better for the MT community! 🌟* 