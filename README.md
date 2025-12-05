## CERVUS x64_86 Operating System

#### Cervus is a modern, 64-bit operating system meticulously crafted for the x8664 architecture. Currently, it's passionately under active development.


### PROJECT STATUS

#### Still cooking! This ain't no finished product, but a journey of pure code magic.


### TODO LIST (THE GRIND CONTINUES!)

### Here's what's keeping us busy, and what's next on our hit list:

- [x] Configure: The initial wien.config configuration is locked and loaded.
- [x] ARCHITECTURE: The preliminary architecture for the wien build system (scripting language) is sketched out.
- [ ] Implement a full-fledged kernel (The real fun begins!)
- [ ] Develop essential drivers (Because hardware needs talking to)
- [ ] Create a basic user-space environment (So we can actually *do* something)
- [ ] Write documentation for the build system (Don't wanna leave anyone guessing)
- [ ] Add more features to wien (Make it even more badass)


### QUICK START (LET'S GET THIS PARTY STARTED!)

#### Wanna get your hands dirty and see Cervus in action? Follow these simple steps.


### PREREQUISITES (WHAT YOU'LL NEED BEFORE YOU BEGIN)

#### Make sure you have these bad boys installed on your system:
```bash
–   x86_64 GCC Compiler & Binutils: For turning our sweet C/C++ code into executable gold.
–   NASM & GAS (GNU Assembler): Because sometimes you just gotta speak directly to the silicon.
–   QEMU: Our trusty virtual machine for booting and testing Cervus without crashing your actual PC.
–   xorriso: For crafting the bootable ISO image.
```

### BUILDING CERVUS (THE MAGIC COMMAND)

#### Our custom build system, `wien`, handles all the heavy lifting. Just execute this command in the project root:
```bash
./wien
```

#### This command will:
```bash
1.  Compile all necessary kernel components.
2.  Assemble the bootloader.
3.  Link everything together.
4.  Generate a bootable cervus.iso image.
5.  Optionally (depending on wien's config), launch Cervus in QEMU immediately.
```