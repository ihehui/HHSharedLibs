
#include "richtexteditorwidget.h"
#include "htmlhighlighter_p.h"


#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QPointer>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>
#include <QtCore/QXmlStreamAttributes>


#include <QtGui>
#include <QtWidgets>




namespace HEHUI {


// Richtext simplification filter helpers: Elements to be discarded
static inline bool filterElement(const QStringRef &name)
{
    return name != QStringLiteral("meta") && name != QStringLiteral("style");
}

// Richtext simplification filter helpers: Filter attributes of elements
static inline void filterAttributes(const QStringRef &name,
                                    QXmlStreamAttributes *atts,
                                    bool *paragraphAlignmentFound)
{
    typedef QXmlStreamAttributes::iterator AttributeIt;

    if (atts->isEmpty())
        return;

     // No style attributes for <body>
    if (name == QStringLiteral("body")) {
        atts->clear();
        return;
    }

    // Clean out everything except 'align' for 'p'
    if (name == QStringLiteral("p")) {
        for (AttributeIt it = atts->begin(); it != atts->end(); ) {
            if (it->name() == QStringLiteral("align")) {
                ++it;
                *paragraphAlignmentFound = true;
            } else {
                it = atts->erase(it);
            }
        }
        return;
    }
}

// Richtext simplification filter helpers: Check for blank QStringRef.
static inline bool isWhiteSpace(const QStringRef &in)
{
    const int count = in.size();
    for (int i = 0; i < count; i++)
        if (!in.at(i).isSpace())
            return false;
    return true;
}

// Richtext simplification filter: Remove hard-coded font settings,
// <style> elements, <p> attributes other than 'align' and
// and unnecessary meta-information.
QString simplifyRichTextFilter(const QString &in, bool *isPlainTextPtr = 0)
{
    unsigned elementCount = 0;
    bool paragraphAlignmentFound = false;
    QString out;
    QXmlStreamReader reader(in);
    QXmlStreamWriter writer(&out);
    writer.setAutoFormatting(true);
    //writer.setAutoFormattingIndent(0);

    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::StartElement:
            elementCount++;
            if (filterElement(reader.name())) {
                const QStringRef name = reader.name();
                QXmlStreamAttributes attributes = reader.attributes();
                filterAttributes(name, &attributes, &paragraphAlignmentFound);
                writer.writeStartElement(name.toString());
                if (!attributes.isEmpty())
                    writer.writeAttributes(attributes);
            } else {
                reader.readElementText(); // Skip away all nested elements and characters.
            }
            break;
        case QXmlStreamReader::Characters:
            if (!isWhiteSpace(reader.text()))
                writer.writeCharacters(reader.text().toString());
            break;
        case QXmlStreamReader::EndElement:
            writer.writeEndElement();
            break;
        default:
            break;
        }
    }
    // Check for plain text (no spans, just <html><head><body><p>)
    if (isPlainTextPtr)
        *isPlainTextPtr = !paragraphAlignmentFound && elementCount == 4u; //
    return out;
}

class RichTextEditor : public QTextEdit
{
    Q_OBJECT
public:
    explicit RichTextEditor(QWidget *parent = 0);
    void setDefaultFont(QFont font);

    QToolBar *createToolBar(QWidget *parent = 0);

    bool simplifyRichText() const      { return m_simplifyRichText; }

public slots:
    void setFontBold(bool b);
    void setFontPointSize(double);
    void setText(const QString &text);
    void setSimplifyRichText(bool v);
    QString text(Qt::TextFormat format) const;

signals:
    void stateChanged();
    void simplifyRichTextChanged(bool);

private:
    bool m_simplifyRichText;
};

class AddLinkDialog : public QDialog
{
    Q_OBJECT

public:
    AddLinkDialog(RichTextEditor *editor, QWidget *parent = 0);
    ~AddLinkDialog();

    int showDialog();

public slots:
    void accept();

private:
    void setupUi();

private:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *titleInput;
    QLabel *label_2;
    QLineEdit *urlInput;
    QSpacerItem *verticalSpacer;
    QFrame *line;
    QDialogButtonBox *buttonBox;

    RichTextEditor *m_editor;
};

AddLinkDialog::AddLinkDialog(RichTextEditor *editor, QWidget *parent) :
    QDialog(parent)
{
    setupUi();

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_editor = editor;
}

AddLinkDialog::~AddLinkDialog()
{

}

int AddLinkDialog::showDialog()
{
    // Set initial focus
    const QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        titleInput->setText(cursor.selectedText());
        urlInput->setFocus();
    } else {
        titleInput->setFocus();
    }

    return exec();
}

void AddLinkDialog::accept()
{
    const QString title = titleInput->text();
    const QString url = urlInput->text();

    if (!title.isEmpty()) {
        QString html = QStringLiteral("<a href=\"");
        html += url;
        html += QStringLiteral("\">");
        html += title;
        html += QStringLiteral("</a>");

        m_editor->insertHtml(html);
    }

    titleInput->clear();
    urlInput->clear();

    QDialog::accept();
}

void AddLinkDialog::setupUi()
{
    setModal(true);
    setWindowTitle(tr("Insert Link"));

    verticalLayout = new QVBoxLayout(this);
    formLayout = new QFormLayout();

    label = new QLabel(tr("Title:"), this);
    formLayout->setWidget(0, QFormLayout::LabelRole, label);

    titleInput = new QLineEdit(this);
    titleInput->setMinimumSize(QSize(337, 0));
    formLayout->setWidget(0, QFormLayout::FieldRole, titleInput);

    label_2 = new QLabel(tr("URL:"), this);
    formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

    urlInput = new QLineEdit(this);
    formLayout->setWidget(1, QFormLayout::FieldRole, urlInput);


    verticalLayout->addLayout(formLayout);

    verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout->addItem(verticalSpacer);

    line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    verticalLayout->addWidget(line);

    buttonBox = new QDialogButtonBox(this);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    verticalLayout->addWidget(buttonBox);

    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

}

class HtmlTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    HtmlTextEdit(QWidget *parent = 0)
        : QTextEdit(parent)
    {}

    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void actionTriggered(QAction *action);
};

void HtmlTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    QMenu *htmlMenu = new QMenu(tr("Insert HTML entity"), menu);

    typedef struct {
        const char *text;
        const char *entity;
    } Entry;

    const Entry entries[] = {
        { "&&amp; (&&)", "&amp;" },
        { "&&nbsp;", "&nbsp;" },
        { "&&lt; (<)", "&lt;" },
        { "&&gt; (>)", "&gt;" },
        { "&&copy; (Copyright)", "&copy;" },
        { "&&reg; (Trade Mark)", "&reg;" },
    };

    for (int i = 0; i < 6; ++i) {
        QAction *entityAction = new QAction(QLatin1String(entries[i].text),
                                            htmlMenu);
        entityAction->setData(QLatin1String(entries[i].entity));
        htmlMenu->addAction(entityAction);
    }

    menu->addMenu(htmlMenu);
    connect(htmlMenu, SIGNAL(triggered(QAction*)), SLOT(actionTriggered(QAction*)));

    menu->exec(event->globalPos());
    delete menu;
}

void HtmlTextEdit::actionTriggered(QAction *action)
{
    insertPlainText(action->data().toString());
}

class ColorAction : public QAction
{
    Q_OBJECT

public:
    ColorAction(QObject *parent);

    const QColor& color() const { return m_color; }
    void setColor(const QColor &color);

signals:
    void colorChanged(const QColor &color);

private slots:
    void chooseColor();

private:
    QColor m_color;
};

ColorAction::ColorAction(QObject *parent):
    QAction(parent)
{
    setText(tr("Text Color"));
    setColor(Qt::black);
    connect(this, SIGNAL(triggered()), this, SLOT(chooseColor()));
}

void ColorAction::setColor(const QColor &color)
{
    if (color == m_color)
        return;
    m_color = color;
    QPixmap pix(24, 24);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(pix.rect(), m_color);
    painter.setPen(m_color.darker());
    painter.drawRect(pix.rect().adjusted(0, 0, -1, -1));
    setIcon(pix);
}

void ColorAction::chooseColor()
{
    const QColor col = QColorDialog::getColor(m_color, 0);
    if (col.isValid() && col != m_color) {
        setColor(col);
        emit colorChanged(m_color);
    }
}

class RichTextEditorToolBar : public QToolBar
{
    Q_OBJECT
public:
    RichTextEditorToolBar(RichTextEditor *editor, QWidget *parent = 0);

public slots:
    void updateActions();

private slots:
    void alignmentActionTriggered(QAction *action);
    void sizeInputActivated(const QString &size);
    void colorChanged(const QColor &color);
    void setVAlignSuper(bool super);
    void setVAlignSub(bool sub);
    void insertLink();
    void insertImage();
    void layoutDirectionChanged();

private:
    QAction *m_bold_action;
    QAction *m_italic_action;
    QAction *m_underline_action;
    QAction *m_valign_sup_action;
    QAction *m_valign_sub_action;
    QAction *m_align_left_action;
    QAction *m_align_center_action;
    QAction *m_align_right_action;
    QAction *m_align_justify_action;
    QAction *m_layoutDirectionAction;
    QAction *m_link_action;
    QAction *m_image_action;
    QAction *m_simplify_richtext_action;
    ColorAction *m_color_action;
    QComboBox *m_font_size_input;

    QPointer<RichTextEditor> m_editor;
};


static QIcon createIconSet(const QString &name){
    return QIcon(":/richtexteditor/resources/images/" + name);
}

static QAction *createCheckableAction(const QIcon &icon, const QString &text,
                                      QObject *receiver, const char *slot,
                                      QObject *parent = 0)
{
    QAction *result = new QAction(parent);
    result->setIcon(icon);
    result->setText(text);
    result->setCheckable(true);
    result->setChecked(false);
    if (slot)
        QObject::connect(result, SIGNAL(triggered(bool)), receiver, slot);
    return result;
}

RichTextEditorToolBar::RichTextEditorToolBar(RichTextEditor *editor, QWidget *parent)
    :QToolBar(parent),
    m_link_action(new QAction(this)),
    m_image_action(new QAction(this)),
    m_color_action(new ColorAction(this)),
    m_font_size_input(new QComboBox),
    m_editor(editor)
{
    // Font size combo box
    m_font_size_input->setEditable(false);
    m_font_size_input->setToolTip(tr("Font Size"));
    const QList<int> font_sizes = QFontDatabase::standardSizes();
    foreach (int font_size, font_sizes)
        m_font_size_input->addItem(QString::number(font_size));

    connect(m_font_size_input, SIGNAL(activated(QString)),
            this, SLOT(sizeInputActivated(QString)));
    addWidget(m_font_size_input);
    addSeparator();

    // Text color button
    connect(m_color_action, SIGNAL(colorChanged(QColor)),
            this, SLOT(colorChanged(QColor)));
    addAction(m_color_action);
    addSeparator();

    // Bold, italic and underline buttons
    m_bold_action = createCheckableAction(
            createIconSet(QStringLiteral("textbold.png")),
            tr("Bold"), editor, SLOT(setFontBold(bool)), this);
    m_bold_action->setShortcut(tr("CTRL+B"));
    addAction(m_bold_action);

    m_italic_action = createCheckableAction(
            createIconSet(QStringLiteral("textitalic.png")),
            tr("Italic"), editor, SLOT(setFontItalic(bool)), this);
    m_italic_action->setShortcut(tr("CTRL+I"));
    addAction(m_italic_action);

    m_underline_action = createCheckableAction(
            createIconSet(QStringLiteral("textunder.png")),
            tr("Underline"), editor, SLOT(setFontUnderline(bool)), this);
    m_underline_action->setShortcut(tr("CTRL+U"));
    addAction(m_underline_action);

    addSeparator();

    // Left, center, right and justified alignment buttons

    QActionGroup *alignment_group = new QActionGroup(this);
    connect(alignment_group, SIGNAL(triggered(QAction*)),
                             SLOT(alignmentActionTriggered(QAction*)));

    m_align_left_action = createCheckableAction(
            createIconSet(QStringLiteral("textleft.png")),
            tr("Left Align"), editor, 0, alignment_group);
    addAction(m_align_left_action);

    m_align_center_action = createCheckableAction(
            createIconSet(QStringLiteral("textcenter.png")),
            tr("Center"), editor, 0, alignment_group);
    addAction(m_align_center_action);

    m_align_right_action = createCheckableAction(
            createIconSet(QStringLiteral("textright.png")),
            tr("Right Align"), editor, 0, alignment_group);
    addAction(m_align_right_action);

    m_align_justify_action = createCheckableAction(
            createIconSet(QStringLiteral("textjustify.png")),
            tr("Justify"), editor, 0, alignment_group);
    addAction(m_align_justify_action);

    m_layoutDirectionAction = createCheckableAction(
            createIconSet(QStringLiteral("righttoleft.png")),
            tr("Right to Left"), this, SLOT(layoutDirectionChanged()));
    addAction(m_layoutDirectionAction);

    addSeparator();

    // Superscript and subscript buttons

    m_valign_sup_action = createCheckableAction(
            createIconSet(QStringLiteral("textsuperscript.png")),
            tr("Superscript"),
            this, SLOT(setVAlignSuper(bool)), this);
    addAction(m_valign_sup_action);

    m_valign_sub_action = createCheckableAction(
            createIconSet(QStringLiteral("textsubscript.png")),
            tr("Subscript"),
            this, SLOT(setVAlignSub(bool)), this);
    addAction(m_valign_sub_action);

    addSeparator();

    // Insert hyperlink and image buttons

    m_link_action->setIcon(createIconSet(QStringLiteral("textanchor.png")));
    m_link_action->setText(tr("Insert &Link"));
    connect(m_link_action, SIGNAL(triggered()), SLOT(insertLink()));
    addAction(m_link_action);

    m_image_action->setIcon(createIconSet(QStringLiteral("insertimage.png")));
    m_image_action->setText(tr("Insert &Image"));
    connect(m_image_action, SIGNAL(triggered()), SLOT(insertImage()));
    addAction(m_image_action);

    addSeparator();

    // Simplify rich text
    m_simplify_richtext_action
        = createCheckableAction(createIconSet(QStringLiteral("simplifyrichtext.png")),
                                tr("Simplify Rich Text"), m_editor, SLOT(setSimplifyRichText(bool)));
    m_simplify_richtext_action->setChecked(m_editor->simplifyRichText());
    connect(m_editor, SIGNAL(simplifyRichTextChanged(bool)),
            m_simplify_richtext_action, SLOT(setChecked(bool)));
    addAction(m_simplify_richtext_action);

    connect(editor, SIGNAL(textChanged()), this, SLOT(updateActions()));
    connect(editor, SIGNAL(stateChanged()), this, SLOT(updateActions()));

    updateActions();
}

void RichTextEditorToolBar::alignmentActionTriggered(QAction *action)
{
    Qt::Alignment new_alignment;

    if (action == m_align_left_action) {
        new_alignment = Qt::AlignLeft;
    } else if (action == m_align_center_action) {
        new_alignment = Qt::AlignCenter;
    } else if (action == m_align_right_action) {
        new_alignment = Qt::AlignRight;
    } else {
        new_alignment = Qt::AlignJustify;
    }

    m_editor->setAlignment(new_alignment);
}

void RichTextEditorToolBar::colorChanged(const QColor &color)
{
    m_editor->setTextColor(color);
    m_editor->setFocus();
}

void RichTextEditorToolBar::sizeInputActivated(const QString &size)
{
    bool ok;
    int i = size.toInt(&ok);
    if (!ok)
        return;

    m_editor->setFontPointSize(i);
    m_editor->setFocus();
}

void RichTextEditorToolBar::setVAlignSuper(bool super)
{
    const QTextCharFormat::VerticalAlignment align = super ?
        QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal;

    QTextCharFormat charFormat = m_editor->currentCharFormat();
    charFormat.setVerticalAlignment(align);
    m_editor->setCurrentCharFormat(charFormat);

    m_valign_sub_action->setChecked(false);
}

void RichTextEditorToolBar::setVAlignSub(bool sub)
{
    const QTextCharFormat::VerticalAlignment align = sub ?
        QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal;

    QTextCharFormat charFormat = m_editor->currentCharFormat();
    charFormat.setVerticalAlignment(align);
    m_editor->setCurrentCharFormat(charFormat);

    m_valign_sup_action->setChecked(false);
}

void RichTextEditorToolBar::insertLink()
{
    AddLinkDialog linkDialog(m_editor, this);
    linkDialog.showDialog();
    m_editor->setFocus();
}

void RichTextEditorToolBar::insertImage()
{
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Select Image"),
                                                QDir::homePath(),
                                                tr("Images (*.png *.bmp *.jpg);;ALl files (*.*)"));

    if (!path.isEmpty())
        m_editor->insertHtml(QStringLiteral("<img src=\"") + path + QStringLiteral("\"/>"));
}

void RichTextEditorToolBar::layoutDirectionChanged()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextBlock block = cursor.block();
    if (block.isValid()) {
        QTextBlockFormat format = block.blockFormat();
        const Qt::LayoutDirection newDirection = m_layoutDirectionAction->isChecked() ? Qt::RightToLeft : Qt::LeftToRight;
        if (format.layoutDirection() != newDirection) {
            format.setLayoutDirection(newDirection);
            cursor.setBlockFormat(format);
        }
    }
}

void RichTextEditorToolBar::updateActions()
{
    if (m_editor == 0) {
        setEnabled(false);
        return;
    }

    const Qt::Alignment alignment = m_editor->alignment();
    const QTextCursor cursor = m_editor->textCursor();
    const QTextCharFormat charFormat = cursor.charFormat();
    const QFont font = charFormat.font();
    const QTextCharFormat::VerticalAlignment valign =
        charFormat.verticalAlignment();
    const bool superScript = valign == QTextCharFormat::AlignSuperScript;
    const bool subScript = valign == QTextCharFormat::AlignSubScript;

    if (alignment & Qt::AlignLeft) {
        m_align_left_action->setChecked(true);
    } else if (alignment & Qt::AlignRight) {
        m_align_right_action->setChecked(true);
    } else if (alignment & Qt::AlignHCenter) {
        m_align_center_action->setChecked(true);
    } else {
        m_align_justify_action->setChecked(true);
    }
    m_layoutDirectionAction->setChecked(cursor.blockFormat().layoutDirection() == Qt::RightToLeft);

    m_bold_action->setChecked(font.bold());
    m_italic_action->setChecked(font.italic());
    m_underline_action->setChecked(font.underline());
    m_valign_sup_action->setChecked(superScript);
    m_valign_sub_action->setChecked(subScript);

    const int size = font.pointSize();
    const int idx = m_font_size_input->findText(QString::number(size));
    if (idx != -1)
        m_font_size_input->setCurrentIndex(idx);

    m_color_action->setColor(m_editor->textColor());
}

RichTextEditor::RichTextEditor(QWidget *parent)
    : QTextEdit(parent), m_simplifyRichText(true)
{
    connect(this, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
            this, SIGNAL(stateChanged()));
    connect(this, SIGNAL(cursorPositionChanged()),
            this, SIGNAL(stateChanged()));
}

QToolBar *RichTextEditor::createToolBar(QWidget *parent)
{
    return new RichTextEditorToolBar(this, parent);
}

void RichTextEditor::setFontBold(bool b)
{
    if (b)
        setFontWeight(QFont::Bold);
    else
        setFontWeight(QFont::Normal);
}

void RichTextEditor::setFontPointSize(double d)
{
    QTextEdit::setFontPointSize(qreal(d));
}

void RichTextEditor::setText(const QString &text)
{

    if (Qt::mightBeRichText(text))
        setHtml(text);
    else
        setPlainText(text);
}

void RichTextEditor::setSimplifyRichText(bool v)
{
    if (v != m_simplifyRichText) {
        m_simplifyRichText = v;
        emit simplifyRichTextChanged(v);
    }
}

void RichTextEditor::setDefaultFont(QFont font)
{
    // Some default fonts on Windows have a default size of 7.8,
    // which results in complicated rich text generated by toHtml().
    // Use an integer value.
    const int pointSize = qRound(font.pointSizeF());
    if (pointSize > 0 && !qFuzzyCompare(qreal(pointSize), font.pointSizeF())) {
        font.setPointSize(pointSize);
    }

    document()->setDefaultFont(font);
    if (font.pointSize() > 0)
        setFontPointSize(font.pointSize());
    else
        setFontPointSize(QFontInfo(font).pointSize());
    emit textChanged();
}

QString RichTextEditor::text(Qt::TextFormat format) const
{
    switch (format) {
    case Qt::PlainText:
        return toPlainText();
    case Qt::RichText:
        return m_simplifyRichText ? simplifyRichTextFilter(toHtml()) : toHtml();
    case Qt::AutoText:
        break;
    }
    const QString html = toHtml();
    bool isPlainText;
    const QString simplifiedHtml = simplifyRichTextFilter(html, &isPlainText);
    if (isPlainText)
        return toPlainText();
    return m_simplifyRichText ? simplifiedHtml : html;
}

RichTextEditorWidget::RichTextEditorWidget(QWidget *parent)
    :QWidget(parent),
    m_richTextEditor(new RichTextEditor()),
    m_sourceCodeEditor(new HtmlTextEdit),
    m_tab_widget(new QTabWidget),
    m_state(Clean),
    m_initialTab(RichTextIndex)
{
    setWindowTitle(tr("Edit text"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_sourceCodeEditor->setAcceptRichText(false);
    m_sourceCodeEditor->setLineWrapMode(QTextEdit::NoWrap);
    new HtmlHighlighter(m_sourceCodeEditor);

    connect(m_richTextEditor, SIGNAL(textChanged()), this, SLOT(richTextChanged()));
    connect(m_richTextEditor, SIGNAL(simplifyRichTextChanged(bool)), this, SLOT(richTextChanged()));
    connect(m_sourceCodeEditor, SIGNAL(textChanged()), this, SLOT(sourceChanged()));

    // The toolbar needs to be created after the RichTextEditor
    QToolBar *tool_bar = m_richTextEditor->createToolBar();
    tool_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QWidget *rich_edit = new QWidget(this);
    QVBoxLayout *rich_edit_layout = new QVBoxLayout(rich_edit);
    rich_edit_layout->addWidget(tool_bar);
    rich_edit_layout->addWidget(m_richTextEditor);

    m_sourceCodeWidget = new QWidget(this);
    QVBoxLayout *plain_edit_layout = new QVBoxLayout(m_sourceCodeWidget);
    plain_edit_layout->addWidget(m_sourceCodeEditor);

    m_tab_widget->setTabBarAutoHide(true);
    m_tab_widget->setTabPosition(QTabWidget::South);
    m_tab_widget->addTab(rich_edit, tr("Rich Text"));
    m_tab_widget->addTab(m_sourceCodeWidget, tr("Source Code"));
    connect(m_tab_widget, SIGNAL(currentChanged(int)), SLOT(tabIndexChanged(int)));


    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_tab_widget);

    m_tab_widget->setCurrentWidget(rich_edit);
    m_richTextEditor->selectAll();
    m_richTextEditor->setFocus();

}

RichTextEditorWidget::~RichTextEditorWidget()
{

}

void RichTextEditorWidget::setDefaultFont(const QFont &font)
{
    m_richTextEditor->setDefaultFont(font);
}

void RichTextEditorWidget::setText(const QString &text)
{
    // Generally simplify rich text unless verbose text is found.
    const bool isSimplifiedRichText = !text.startsWith(QStringLiteral("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"));
    m_richTextEditor->setSimplifyRichText(isSimplifiedRichText);
    m_richTextEditor->setText(text);
    m_sourceCodeEditor->setPlainText(text);
    m_state = Clean;
}

QString RichTextEditorWidget::text(bool compactSRC, Qt::TextFormat format) const
{
    // In autotext mode, if the user has changed the source, use that
    if (format == Qt::AutoText && (m_state == Clean || m_state == SourceChanged))
        return m_sourceCodeEditor->toPlainText();
    // If the plain text HTML editor is selected, first copy its contents over
    // to the rich text editor so that it is converted to Qt-HTML or actual
    // plain text.
    if (m_tab_widget->currentIndex() == SourceIndex && m_state == SourceChanged)
        m_richTextEditor->setHtml(m_sourceCodeEditor->toPlainText());

    if(compactSRC){
        return formatHTML(m_richTextEditor->text(format), compactSRC);
    }

    return m_richTextEditor->text(format);
}

void RichTextEditorWidget::setSourceCodeEditEnabled(bool enabled){
    int idx = m_tab_widget->indexOf(m_sourceCodeWidget);
    if(idx == -1){
        if(enabled){
            m_tab_widget->addTab(m_sourceCodeWidget, tr("Source Code"));
        }
    }else{
        if(!enabled){
            m_tab_widget->removeTab(idx);
        }
    }

}

void RichTextEditorWidget::tabIndexChanged(int newIndex)
{
    // Anything changed, is there a need for a conversion?
    if (newIndex == SourceIndex && m_state != RichTextChanged)
        return;
    if (newIndex == RichTextIndex && m_state != SourceChanged)
        return;
    const State oldState = m_state;
    // Remember the cursor position, since it is invalidated by setPlainText
    QTextEdit *new_edit = (newIndex == SourceIndex) ? m_sourceCodeEditor : m_richTextEditor;
    const int position = new_edit->textCursor().position();

    if (newIndex == SourceIndex)
        m_sourceCodeEditor->setPlainText(formatHTML(m_richTextEditor->text(Qt::RichText)));
    else
        m_richTextEditor->setHtml(m_sourceCodeEditor->toPlainText());

    QTextCursor cursor = new_edit->textCursor();
    cursor.movePosition(QTextCursor::End);
    if (cursor.position() > position) {
        cursor.setPosition(position);
    }
    new_edit->setTextCursor(cursor);
    m_state = oldState; // Changed is triggered by setting the text

}

void RichTextEditorWidget::richTextChanged()
{
    m_state = RichTextChanged;
}

void RichTextEditorWidget::sourceChanged()
{
    m_state = SourceChanged;
}

QString RichTextEditorWidget::formatHTML(const QString &html, bool compactSRC) const{

    QString out;
    QXmlStreamReader reader(html);
    QXmlStreamWriter writer(&out);
    if(compactSRC){
        writer.setAutoFormatting(false);
        writer.setAutoFormattingIndent(0);
    }else{
        writer.setAutoFormatting(true);
    }

    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::StartElement:
        {
            const QStringRef name = reader.name();
            QXmlStreamAttributes attributes = reader.attributes();
            writer.writeStartElement(name.toString());
            if (!attributes.isEmpty())
                writer.writeAttributes(attributes);
        }
            break;
        case QXmlStreamReader::Characters:
            if (!isWhiteSpace(reader.text()))
                writer.writeCharacters(reader.text().toString());
            break;
        case QXmlStreamReader::EndElement:
            writer.writeEndElement();
            break;
        default:
            break;
        }
    }

    return out;

}




} // namespace HEHUI

#include "richtexteditorwidget.moc"
