// For license of this file, see <project-root-folder>/LICENSE.md.

#include "definitions/definitions.h"
#include "dynamic-shortcuts/dynamicshortcuts.h"
#include "gui/dialogs/formabout.h"
#include "gui/dialogs/formmain.h"
#include "gui/dialogs/formupdate.h"
#include "gui/messagebox.h"
#include "miscellaneous/application.h"
#include "miscellaneous/debugging.h"
#include "miscellaneous/iconfactory.h"
#include "miscellaneous/textapplication.h"
#include "network-web/silentnetworkaccessmanager.h"
#include "network-web/webfactory.h"

// Needed for setting ini file format on Mac OS.
#ifdef Q_OS_MAC
#include <QSettings>
#endif

#include <QDebug>
#include <QThread>
#include <QTranslator>

#if defined (Q_OS_MAC)
extern void disableWindowTabbing();

#endif

#include "hunspell/hunspell.hxx"

int main(int argc, char* argv[]) {
  //: Abbreviation of language, e.g. en.
  //: Use ISO 639-1 code here combined with ISO 3166-1 (alpha-2) code.
  //: Examples: "cs", "en", "it", "cs_CZ", "en_GB", "en_US".
  QObject::tr("LANG_ABBREV");

  // Setup debug output system.
  qInstallMessageHandler(Debugging::debugHandler);

  // Instantiate base application object.
  Application application(APP_LOW_NAME, argc, argv);

  // hunspell test
  Hunspell spl("C:\\Projekty\\cs_CZ.aff", "C:\\Projekty\\cs_CZ.dic");
  bool aaa = spl.spell("autíčki");
  QStringList suggestions;
  char** suggestWordList = NULL;

  // Encode from Unicode to the encoding used by current dictionary
  int count = spl.suggest(&suggestWordList, "autíčki");

  for (int i = 0; i < count; ++i) {
    QString suggestion(suggestWordList[i]);

    suggestions << suggestion;
    free(suggestWordList[i]);
  }

  qDebug("Instantiated Application class.");

  // Check if another instance is running.
  if (application.sendMessage((QStringList() << APP_IS_RUNNING << application.arguments().mid(1)).join(ARGUMENTS_LIST_SEPARATOR))) {
    qWarning("Another instance of the application is already running. Notifying it.");
    return EXIT_FAILURE;
  }

  Application::setAttribute(Qt::AA_UseHighDpiPixmaps);
  Application::setAttribute(Qt::AA_EnableHighDpiScaling);
  Application::setApplicationName(APP_NAME);
  Application::setApplicationVersion(APP_VERSION);
  Application::setOrganizationDomain(APP_URL);
  Application::setWindowIcon(QIcon(APP_ICON_PATH));

#if defined (Q_OS_MAC)
  Application::setAttribute(Qt::AA_DontShowIconsInMenus);
  disableWindowTabbing();
#endif

  qApp->localization()->loadActiveLanguage();
  qApp->icons()->setupSearchPaths();
  qApp->icons()->loadIconTheme(qApp->settings()->value(GROUP(GUI), SETTING(GUI::IconTheme)).toString());
  qApp->setStyle(qApp->settings()->value(GROUP(GUI), SETTING(GUI::Style)).toString());

  // Setup single-instance behavior.
  QObject::connect(&application, &Application::messageReceived, &application, &Application::processExecutionMessage);
  qDebug().nospace() << "Creating main application form in thread: \'" << QThread::currentThreadId() << "\'.";

  // Instantiate main application window.
  FormMain main_window;

  // Set correct information for main window.
  main_window.setWindowTitle(APP_LONG_NAME);

  // Display main window.
  qDebug("Showing the main window when the application is starting.");
  main_window.show();

  if (qApp->isFirstRun(APP_VERSION)) {
    qApp->showGuiMessage(QObject::tr("Welcome to %1. Click on me to check out NEW features.").arg(APP_LONG_NAME),
                         QMessageBox::Icon::Information, QUrl("http://update.textosaurus"),
                         [&main_window]() {
      FormAbout(&main_window).exec();
    });
  }

  // We load any documents passed as parameters.
  if (application.arguments().size() > 1) {
    qApp->textApplication()->loadFilesFromArgs(application.arguments().mid(1));
  }
  else {
    qApp->textApplication()->newFile();
  }

  // Now is a good time to initialize dynamic keyboard shortcuts.
  DynamicShortcuts::load(qApp->userActions());

  // Enter global event loop.
  return Application::exec();
}
