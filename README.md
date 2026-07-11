# OJclicks OS

Look, this is a 32-bit x86 operating system and kernel written from scratch in C and Assembly. The system has a future vision that it might be rewritten all over again from scratch using a programming language of my own invention too... For now, at least, the system remains strictly text-based. So, your experience here will be pretty much like someone looking for water in a desert.

The whole concept of the system, in short: fighting user surveillance and data interference under the pretext of "improving the user experience." This is a system I designed from the ground up specifically for this goal—no spying on anyone, no selling their data, and none of that nonsense. And, of course, to accelerate and optimize performance even on dead hardware... pre-Windows era.

If you don't understand how GDT, IDT, or Paging works, I advise you to go read the Linux source code... that is, if you actually want to help with the development.

## Current Features

* Custom 32-bit Kernel entry (because GRUB is bloated, mainstream, and everyone uses it).
* GDT, IDT, and Paging initialized from scratch (if you still rely on standard BIOS interrupts... go download Windows 11).
* Programmable Interrupt Controller (PIC) and a keyboard driver that actually works.
* ATA Hard Disk drive driver and a basic file system (don't expect ext4 right now...).
* Physical Memory Manager (PMM) and Virtual Memory Heap for proper memory isolation and management.
* Initial Usermode architecture and basic System Calls (Syscalls).
* A custom C Shell (ojcsh) to test all this chaos without losing your mind.

## Building and Running

If you are working on a broken, bloated operating system, throw it away and go install Linux (any lightweight distro at least) first. You will need these tools: `nasm`, `gcc` (configured for i686-elf targets, don't use your host compiler like an amateur), and `make`.

To compile the kernel and build the disk image, run:

```bash
make clean
make

```

If translation or compilation errors pop up, it's 99% your environment's fault and your Toolchain, not my code. Fix your tools.

To test the generated `disk.img` using QEMU:

```bash
dd if=/dev/zero of=hdd.img bs=1M count=10

cat build/boot.bin build/kernel.bin > disk.img && truncate -s 1474560 disk.img

qemu-system-i386 -fda disk.img -hda hdd.img 

```

If this command crashes your machine, congratulations, you just learned what low-level systems programming actually means.

# .
# عربي

# OJclicks OS

بص، دا نظام تشغيل ونواة 32-بت لمعمارية x86 مكتوب من الصفر بلغة C والـ Assembly. والنظام له رؤية مقبلة انه جايز يتكتب تاني من الصفر بلغة برمجه من اختراعي بردو... ولا النظام مازال (text-based)الى الان على الاقل... فتجربتك هتبقى عاملة زي اللي عايز ماية في صحراء.
فكرة النظام بكل اختصار... محاربة مراقبة الناس والتخدل في البيانات بحجة تحسين الاستخادم... ودا نظام رسمته في الاساس لهذا الهدف بدون التطفل على حد ولا بيع بياناته ولا ايه حاجه من الهجس دا كله واكيد تسريع وتحسين الاستخدام حتى على الاجهزة الميته... ما قبل ويندوز...

لو مش فاهم الـ GDT أو الـ IDT أو الـ Paging بيشتغلوا ازاي، انصحك تقرا كود لينكس... دا لو عايز تساعد في البناء...

## الميزات الحالية

* إدخال نواة مخصص 32-بت (لأن GRUB منفوخ ومبتذل وكل الناس بتستخدمه).
* تهيئة الـ GDT والـ IDT والـ Paging من الصفر (لو بتعتمد على مقاطعات بيوس القياسية... روح حمل ويندوز 11).
* متحكم المقاطعات المبرمج (PIC) ومشغل لوحة مفاتيح شغال فعلياً.
* مشغل أقراص ATA ونظام ملفات أساسي (متتوقعش ext4 دلوقتي...).
* مدير الذاكرة الفيزيائية (PMM) ومكدس الذاكرة الافتراضية (Heap) لعزل وإدارة الذاكرة.
* بناء وضع المستخدم (Usermode) الأولي واستدعاءات النظام (Syscalls).
* شل مخصص بلغة C اسمه (ojcsh) عشان تختبر العك ده كله من غير ما تفقد عقلك.

## البناء والتشغيل

لو أنت شغال على نظام تشغيل مكسور ومنفوخ، ارميه وروح ثبت linux (اي توزيعه خفيفة حتى) الأول. هتحتاج الأدوات دي: `nasm` و `gcc` (يكون مهيأ لأهداف i686-elf، متستخدمش الكومبايلر الأساسي بتاع جهازك زي الهواة) و `make`.

عشان تترجم النواة وتبني صورة القرص، شغل:

```bash
make clean
make
```

لو ظهرت لك أخطاء أثناء الترجمة، فبنسبة 99% العيب في البيئة والـ Toolchain بتاعتك مش في الكود بتاعي. ظبط أدواتك.

لاختبار ملف disk.img الناتج باستخدام QEMU:
```bash
dd if=/dev/zero of=hdd.img bs=1M count=10

cat build/boot.bin build/kernel.bin > disk.img && truncate -s 1474560 disk.img

qemu-system-i386 -fda disk.img -hda hdd.img 
```
لو الأمر ده عمل كراش لجهازك، مبروك، أنت لسه متعلم حالا يعني إيه برمجة الأنظمة منخفضة المستوى.

