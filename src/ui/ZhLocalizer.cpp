#include "ui/ZhLocalizer.h"

#include <QAbstractButton>
#include <QAbstractItemModel>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QGroupBox>
#include <QHash>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QSet>
#include <QTabWidget>
#include <QTimer>
#include <QVariant>
#include <QWidget>

#include <functional>

namespace compositor::ui {
namespace {

// English -> Simplified Chinese. Keys are exact UI strings as produced by the
// application; values are the Chinese display text. Data keys / persistence
// identifiers are deliberately NOT touched (they live in the code, not here).
const char* const kPairs[][2] = {
    // ---- Menu bar ----
    {"File", "文件"},
    {"Edit", "编辑"},
    {"Clipboard", "剪贴板"},
    {"Select", "选择"},
    {"Image", "图像"},
    {"View", "视图"},
    {"Canvas", "画布"},
    {"Layer", "图层"},
    {"Transform", "变换"},
    {"Transform Options", "变换选项"},
    {"Adjustments", "调整"},
    {"Tools", "工具"},
    {"Drawing Options", "绘制选项"},
    {"Filters", "滤镜"},
    {"Help", "帮助"},

    // ---- File / app commands ----
    {"New Canvas…", "新建画布…"},
    {"Open Project…", "打开项目…"},
    {"Import Image…", "导入图像…"},
    {"Export Image…", "导出图像…"},
    {"Save Project", "保存项目"},
    {"Save Project As…", "项目另存为…"},
    {"Close Project", "关闭项目"},
    {"Exit", "退出"},
    {"About Compositor", "关于 Compositor"},
    {"Check for Updates…", "检查更新…"},

    // ---- Edit / clipboard ----
    {"Undo", "撤销"},
    {"Redo", "重做"},
    {"Cut", "剪切"},
    {"Copy", "复制"},
    {"Paste", "粘贴"},
    {"Copy Merged", "合并复制"},
    {"Layer via Copy", "通过拷贝新建图层"},
    {"Clear", "清除"},

    // ---- Select ----
    {"All", "全选"},
    {"Deselect", "取消选择"},
    {"Inverse", "反选"},
    {"Expand…", "扩展…"},
    {"Contract…", "收缩…"},

    // ---- Image / View / Canvas ----
    {"Fill…", "填充…"},
    {"Invert", "反相"},
    {"Fit Canvas", "适合窗口"},
    {"Actual Pixels", "实际像素"},
    {"Pixel Grid", "像素网格"},
    {"Canvas Size…", "画布大小…"},
    {"Image Size…", "图像大小…"},
    {"Flip Canvas Horizontally", "水平翻转画布"},
    {"Flip Canvas Vertically", "垂直翻转画布"},
    {"Crop Tool", "裁剪工具"},
    {"Apply Crop", "应用裁剪"},
    {"Cancel Crop", "取消裁剪"},

    // ---- Layer ----
    {"New Layer", "新建图层"},
    {"Duplicate Layer", "复制图层"},
    {"Rename Layer…", "重命名图层…"},
    {"Delete Layer", "删除图层"},
    {"Raise Layer", "上移图层"},
    {"Lower Layer", "下移图层"},
    {"New Group", "新建组"},
    {"Group Selected", "编组所选"},
    {"Move Out of Group", "移出组"},
    {"Create / Release Clipping Mask", "创建／释放剪贴蒙版"},
    {"Merge Layers", "合并图层"},
    {"Copy Layer to Project", "拷贝图层到项目"},
    {"Copy Layer to Project…", "拷贝图层到项目…"},
    {"Layers", "图层"},

    // ---- Transform ----
    {"Free Transform", "自由变换"},
    {"Distort", "扭曲"},
    {"Apply Transform", "应用变换"},
    {"Cancel Transform", "取消变换"},
    {"Flip Layer Horizontally", "水平翻转图层"},
    {"Flip Layer Vertically", "垂直翻转图层"},
    {"Scale…", "缩放…"},
    {"Flip H", "水平翻转"},
    {"Flip V", "垂直翻转"},

    // ---- Adjustments / filters (menus & dialogs) ----
    {"Adjustments", "调整"},
    {"Edit Adjustment Layer…", "编辑调整图层…"},
    {"New Adjustment Layer", "新建调整图层"},
    {"Hue/Saturation", "色相／饱和度"},
    {"Hue/Saturation…", "色相／饱和度…"},
    {"Levels", "色阶"},
    {"Levels…", "色阶…"},
    {"Curves", "曲线"},
    {"Curves…", "曲线…"},
    {"Exposure", "曝光"},
    {"Exposure…", "曝光…"},
    {"Gradient Map", "渐变映射"},
    {"Gradient Map…", "渐变映射…"},
    {"Grain", "颗粒"},
    {"Grain…", "颗粒…"},
    {"Gaussian Blur", "高斯模糊"},
    {"Gaussian Blur…", "高斯模糊…"},
    {"Motion Blur", "动感模糊"},
    {"Motion Blur…", "动感模糊…"},
    {"Add Noise", "添加杂色"},
    {"Add Noise…", "添加杂色…"},
    {"Lens Correction", "镜头校正"},
    {"Lens Correction…", "镜头校正…"},
    {"Content-Aware Fill", "内容识别填充"},
    {"Content-Aware Fill…", "内容识别填充…"},
    {"Remove Background", "移除背景"},
    {"Remove Background…", "移除背景…"},

    // ---- Tools (toolbar / tool menu) ----
    {"Move (V)", "移动 (V)"},
    {"Hand (H)", "抓手 (H)"},
    {"Marquee (M)", "矩形选框 (M)"},
    {"Lasso (L)", "套索 (L)"},
    {"Polygon", "多边形套索"},
    {"Wand (W)", "魔棒 (W)"},
    {"Brush (B)", "画笔 (B)"},
    {"Eraser (E)", "橡皮擦 (E)"},
    {"Clone (S)", "仿制图章 (S)"},
    {"Heal (J)", "修复 (J)"},
    {"Retouch (R)", "润饰 (R)"},
    {"Gradient (G)", "渐变 (G)"},
    {"Shape (U)", "形状 (U)"},
    {"Crop (C)", "裁剪 (C)"},
    {"Eyedropper (I)", "吸管 (I)"},
    {"Zoom (Z)", "缩放 (Z)"},

    // ---- Tools menu: color well actions ----
    {"Foreground", "前景"},
    {"Background", "背景"},
    {"Foreground color", "前景色"},
    {"Background color", "背景色"},
    {"Swap (X)", "交换 (X)"},
    {"Default (D)", "默认 (D)"},
    {"Swap colors", "交换颜色"},
    {"Default colors", "默认颜色"},
    {"Swap foreground and background (X)", "交换前景色和背景色 (X)"},
    {"Default colors (D)", "默认颜色 (D)"},

    // ---- Drawing options ----
    {"Apply Gradient", "应用渐变"},
    {"Cancel Gradient", "取消渐变"},
    {"Update Shape Style", "更新形状样式"},

    // ---- Blend modes ----
    {"Normal", "正常"},
    {"Multiply", "正片叠底"},
    {"Screen", "滤色"},
    {"Overlay", "叠加"},
    {"Darken", "变暗"},
    {"Lighten", "变亮"},
    {"Difference", "差值"},
    {"Color Dodge", "颜色减淡"},
    {"Color Burn", "颜色加深"},
    {"Hue", "色相"},
    {"Saturation", "饱和度"},
    {"Color", "颜色"},
    {"Luminosity", "明度"},

    // ---- Mask commands ----
    {"Add White Mask (Hide Selection)", "添加白色蒙版（隐藏选区）"},
    {"Add Black Mask (Reveal Selection)", "添加黑色蒙版（显示选区）"},
    {"Reveal All", "全部显示"},
    {"Hide All", "全部隐藏"},
    {"Enable / Disable Mask", "启用／停用蒙版"},
    {"Link / Unlink Mask", "链接／取消链接蒙版"},
    {"Delete Mask", "删除蒙版"},
    {"Edit Image", "编辑图像"},
    {"Edit Mask", "编辑蒙版"},
    {"Edit image", "编辑图像"},
    {"Edit mask", "编辑蒙版"},
    {"Select Image Alpha", "选择图像透明区域"},
    {"Select Mask Black Areas", "选择蒙版黑色区域"},
    {"Paint mask white", "将蒙版涂成白色"},

    // ---- Layer panel header ----
    {"Image", "图像"},
    {"Mask", "蒙版"},
    {"Link", "链接"},

    // ---- Combo items: channels / units / anchor / sampling / ratios ----
    {"Red", "红"},
    {"Green", "绿"},
    {"Blue", "蓝"},
    {"Pixels", "像素"},
    {"Percent", "百分比"},
    {"Inches", "英寸"},
    {"Centimeters", "厘米"},
    {"Top left", "左上"},
    {"Top", "上"},
    {"Top right", "右上"},
    {"Left", "左"},
    {"Center", "居中"},
    {"Right", "右"},
    {"Bottom left", "左下"},
    {"Bottom", "下"},
    {"Bottom right", "右下"},
    {"Nearest", "最近邻"},
    {"Smooth", "平滑"},
    {"High quality", "高质量"},
    {"Free", "自由"},
    {"Original", "原始"},
    {"Point Sample", "点取样"},
    {"3 by 3 Average", "3×3 平均"},
    {"5 by 5 Average", "5×5 平均"},
    {"New selection", "新选区"},
    {"Add", "相加"},
    {"Subtract", "相减"},
    {"Freehand", "手绘"},
    {"Polygonal", "多边形"},
    {"Rectangle", "矩形"},
    {"Ellipse", "椭圆"},
    {"Linear", "线性"},
    {"Radial", "径向"},
    {"Foreground to Background", "前景色到背景色"},
    {"Foreground to Transparent", "前景色到透明"},
    {"Content-Aware", "内容识别"},
    {"Create Texture", "创建纹理"},
    {"Proximity Match", "邻近匹配"},
    {"Liquify", "液化"},
    {"Blur", "模糊"},
    {"Smudge", "涂抹"},
    {"Master", "全图"},
    {"Reds", "红色"},
    {"Yellows", "黄色"},
    {"Greens", "绿色"},
    {"Cyans", "青色"},
    {"Blues", "蓝色"},
    {"Magentas", "洋红色"},

    // ---- Dialog rows / labels ----
    {"Anchor", "锚点"},
    {"Channel", "通道"},
    {"Range", "范围"},
    {"Format", "格式"},
    {"Units", "单位"},
    {"Resampling", "重采样"},
    {"Encoded size", "编码后大小"},
    {"Height", "高度"},
    {"Height (pixels)", "高度（像素）"},
    {"Width", "宽度"},
    {"Width (pixels)", "宽度（像素）"},
    {"Resolution (pixels/inch)", "分辨率（像素/英寸）"},
    {"JPEG background", "JPEG 背景"},
    {"JPEG quality", "JPEG 质量"},
    {"Amount", "数量"},
    {"Angle", "角度"},
    {"Radius", "半径"},
    {"Distance", "距离"},
    {"Distortion", "扭曲"},
    {"Contrast", "对比度"},
    {"Hardness", "硬度"},
    {"Opacity", "不透明度"},
    {"Size", "大小"},
    {"Quality", "质量"},
    {"Preview", "预览"},
    {"Refine", "细化"},
    {"Shift Edge", "移动边缘"},
    {"Reverse", "反向"},
    {"Input black", "输入黑场"},
    {"Gamma", "伽马"},
    {"Input white", "输入白场"},
    {"Output black", "输出黑场"},
    {"Output white", "输出白场"},
    {"Basic", "基本"},
    {"Advanced", "高级"},
    {"Sample All Layers", "取样全部图层"},
    {"Sample original", "取样原图"},
    {"Original image", "原始图像"},
    {"Aligned", "对齐"},
    {"Antialias", "消除锯齿"},
    {"Contiguous", "连续"},
    {"Gaussian", "高斯"},
    {"Monochromatic", "单色"},
    {"Colorize", "着色"},
    {"Sample color", "取样颜色"},
    {"Add color", "添加颜色"},
    {"Remove color", "删除颜色"},
    {"Lightness", "明度"},
    {"Color + neutral midtones", "颜色 + 中性中间调"},
    {"Neutral", "中性"},

    // ---- Transform options / snapping ----
    {"Lock aspect ratio", "锁定纵横比"},
    {"Auto-select layer", "自动选择图层"},
    {"Show controls", "显示控件"},
    {"Snap to edges and centers", "对齐到边缘和中心"},
    {"Targeted adjustment", "目标调整"},

    // ---- Canvas interaction hints ----
    {"Click the canvas to sample", "点击画布取样"},
    {"Click to add a point. Drag to adjust.", "点击添加控制点，拖动进行调整。"},
    {"Remove point", "删除控制点"},

    // ---- Welcome / project open dialog ----
    {"New canvas", "新建画布"},
    {"New canvas · Drop images or layers here for new projects", "新建画布 · 将图像或图层拖到此处可创建新项目"},
    {"Open project", "打开项目"},
    {"Home", "主目录"},
    {"Parent folder", "上级文件夹"},
    {"Select a .comp project. Double-click folders to browse.", "选择一个 .comp 项目。双击文件夹可浏览。"},
    {"Choose a folder outside a .comp project.", "请选择一个 .comp 项目之外的文件夹。"},

    // ---- Appearance / chrome ----
    {"Appearance", "外观"},
    {"Mac-style title bar", "Mac 风格标题栏"},
    {"Use colored window controls on the left. Turn off for the native Windows title bar.", "在左侧使用彩色窗口控件。关闭则使用原生 Windows 标题栏。"},
    {"Transparent canvas · sRGB", "透明画布 · sRGB"},

    // ---- Hue / saturation extras ----
    {"Apply outside this range instead", "改为应用到此范围以外"},
    {"Reset Hue/Saturation", "重置色相／饱和度"},
    {"Reset Levels", "重置色阶"},
    {"Reset curve", "重置曲线"},

    // ---- Progress / status / messages ----
    {"Updating preview…", "正在更新预览…"},
    {"Applying mask…", "正在应用蒙版…"},
    {"Finding foreground…", "正在查找前景…"},
    {"Preparing update check…", "正在准备检查更新…"},
    {"Cancelling the update operation…", "正在取消更新操作…"},
    {"Verifying files and testing the installed version…", "正在校验文件并测试已安装的版本…"},
    {"Waiting for the image provider to finish cancelling…", "正在等待图像提供程序完成取消…"},
    {"Restart cancelled. The installed update remains ready for the next launch.", "已取消重启。已安装的更新将在下次启动时生效。"},
    {"Cannot display export preview", "无法显示导出预览"},
    {"Background removal could not be completed. Try a smaller image or reopen the app.", "背景移除未能完成。请尝试较小的图像或重新打开应用。"},
    {"Background removal could not load its runtime. Repair or reinstall Compositor.", "背景移除无法加载其运行时。请修复或重新安装 Compositor。"},
    {"No foreground subject was detected. Try an image with a more distinct subject.", "未检测到前景主体。请尝试主体更清晰的图像。"},
    {"Works best with a distinct, opaque subject. Transparent objects, fine hair and fur may need manual mask cleanup.", "最适合轮廓清晰的不透明主体。透明物体、细发和毛发可能需要手动修整蒙版。"},
    {"Histogram weighted by opacity and selection", "按不透明度与选区加权的直方图"},
    {"Linear histogram with automatic vertical scaling. Tall spikes may extend beyond the graph; all tones from 0 to 255 remain included.", "线性直方图，自动纵向缩放。高峰可能超出图表范围；0 到 255 的所有色阶仍全部计入。"},

    // ---- Unit fragments (dynamic suffixes) ----
    {" px", " 像素"},
    {" %", " %"},
    {" degrees", " 度"},
    {" percent", " 百分比"},
};

const QHash<QString, QString>& dictionary() {
    static const QHash<QString, QString> table = [] {
        QHash<QString, QString> m;
        m.reserve(int(sizeof(kPairs) / sizeof(kPairs[0])));
        for (const auto& pair : kPairs) m.insert(QString::fromUtf8(pair[0]), QString::fromUtf8(pair[1]));
        return m;
    }();
    return table;
}

QString translate(const QString& source) {
    if (source.isEmpty()) return QString();
    const QHash<QString, QString>& table = dictionary();

    // Preserve "&" mnemonics: "&File" -> "文件(&F)".
    const int amp = source.indexOf(QLatin1Char('&'));
    if (amp >= 0) {
        QString key = source;
        key.remove(QLatin1Char('&'));
        const auto it = table.find(key);
        if (it != table.end() && amp + 1 < source.size())
            return it.value() + QStringLiteral("(&") + source.mid(amp + 1, 1).toUpper() + QLatin1Char(')');
    }

    const auto it = table.find(source);
    if (it != table.end()) return it.value();

    // Dynamic composites like "Undo <history name>" / "Hide <panel>".
    struct Prefix { const char* from; const char* to; };
    static const Prefix prefixes[] = {
        {"Undo ", "撤销 "},
        {"Redo ", "重做 "},
        {"Hide ", "隐藏 "},
        {"Show ", "显示 "},
    };
    for (const Prefix& prefix : prefixes) {
        const QString from = QString::fromUtf8(prefix.from);
        if (source.startsWith(from)) {
            const QString rest = source.mid(from.size());
            const QString translatedRest = translate(rest);
            return QString::fromUtf8(prefix.to) + (translatedRest.isEmpty() ? rest : translatedRest);
        }
    }
    return QString();
}

void applyIfTranslated(const QString& current, const std::function<void(const QString&)>& setter) {
    const QString zh = translate(current);
    if (!zh.isEmpty() && zh != current) setter(zh);
}

class Localizer final : public QObject {
public:
    explicit Localizer(QObject* parent) : QObject(parent) {
        if (qApp) qApp->installEventFilter(this);
        timer_.setInterval(300);
        connect(&timer_, &QTimer::timeout, this, [this] { sweep(); });
        timer_.start();
        sweep();
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        switch (event->type()) {
        case QEvent::Show:
        case QEvent::Polish:
        case QEvent::ChildAdded:
        case QEvent::LayoutRequest:
        case QEvent::LanguageChange:
        case QEvent::DynamicPropertyChange:
            seen_.clear();
            if (auto* widget = qobject_cast<QWidget*>(watched)) translateWidget(widget);
            else if (auto* action = qobject_cast<QAction*>(watched)) translateAction(action);
            break;
        default:
            break;
        }
        return false;
    }

private:
    void sweep() {
        seen_.clear();
        if (!qApp) return;
        const auto tops = QApplication::topLevelWidgets();
        for (QWidget* widget : tops) translateWidget(widget);
    }

    void translateAction(QAction* action) {
        if (!action || seen_.contains(action)) return;
        seen_.insert(action);
        applyIfTranslated(action->text(), [action](const QString& zh) { action->setText(zh); });
        if (!action->toolTip().isEmpty()) applyIfTranslated(action->toolTip(), [action](const QString& zh) { action->setToolTip(zh); });
        if (!action->statusTip().isEmpty()) applyIfTranslated(action->statusTip(), [action](const QString& zh) { action->setStatusTip(zh); });
        if (QMenu* menu = action->menu())
            for (QAction* child : menu->actions()) translateAction(child);
    }

    void translateWidget(QWidget* widget) {
        if (!widget || seen_.contains(widget)) return;
        seen_.insert(widget);

        if (widget->isWindow() && !widget->windowTitle().isEmpty())
            applyIfTranslated(widget->windowTitle(), [widget](const QString& zh) { widget->setWindowTitle(zh); });

        if (auto* button = qobject_cast<QAbstractButton*>(widget))
            applyIfTranslated(button->text(), [button](const QString& zh) { button->setText(zh); });

        if (auto* group = qobject_cast<QGroupBox*>(widget))
            applyIfTranslated(group->title(), [group](const QString& zh) { group->setTitle(zh); });

        if (auto* label = qobject_cast<QLabel*>(widget)) {
            const QString text = label->text();
            if (!text.contains(QLatin1Char('<')) && !text.contains(QLatin1Char('&')))
                applyIfTranslated(text, [label](const QString& zh) { label->setText(zh); });
        }

        if (auto* line = qobject_cast<QLineEdit*>(widget))
            applyIfTranslated(line->placeholderText(), [line](const QString& zh) { line->setPlaceholderText(zh); });

        if (auto* combo = qobject_cast<QComboBox*>(widget)) {
            for (int i = 0; i < combo->count(); ++i) {
                if (combo->itemData(i).isValid()) continue;
                const QString item = combo->itemText(i);
                const QString zh = translate(item);
                if (!zh.isEmpty() && zh != item) combo->setItemText(i, zh);
            }
        }

        if (auto* tabs = qobject_cast<QTabWidget*>(widget)) {
            for (int i = 0; i < tabs->count(); ++i) {
                const QString tab = tabs->tabText(i);
                const QString zh = translate(tab);
                if (!zh.isEmpty() && zh != tab) tabs->setTabText(i, zh);
            }
        }

        if (auto* header = qobject_cast<QHeaderView*>(widget)) {
            if (QAbstractItemModel* model = header->model()) {
                for (int i = 0; i < model->columnCount(); ++i) {
                    const QString text = model->headerData(i, header->orientation()).toString();
                    const QString zh = translate(text);
                    if (!zh.isEmpty() && zh != text) model->setHeaderData(i, header->orientation(), zh);
                }
            }
        }

        if (!widget->toolTip().isEmpty())
            applyIfTranslated(widget->toolTip(), [widget](const QString& zh) { widget->setToolTip(zh); });

        for (QAction* action : widget->actions()) translateAction(action);
        for (QObject* child : widget->children()) {
            if (auto* childWidget = qobject_cast<QWidget*>(child)) translateWidget(childWidget);
            else if (auto* childAction = qobject_cast<QAction*>(child)) translateAction(childAction);
        }
    }

    QTimer timer_;
    QSet<const QObject*> seen_;
};

}  // namespace

QObject* installSimplifiedChineseLocalizer(QObject* parent) {
    return new Localizer(parent ? parent : (qApp ? static_cast<QObject*>(qApp) : nullptr));
}

}  // namespace compositor::ui
