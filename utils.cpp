/*
this code is Public Domain
Peter Semiletov
*/



#include <QTextStream>
#include <QTextCodec>
#include <QDebug>
#include <QDir>
#include <QImageReader>
#include <QImage>
#include <QProcess>

#include "utils.h"


/* file utils */

QString guess_enc_for_file (const QString &fname)
{
  QString enc = "UTF-8";

  QProcess p;
  p.start ("enca", QStringList() << "-i" << fname);

  if (! p.waitForStarted() || ! p.waitForFinished() )
     return "err";

  QString s = p.readAllStandardOutput();
  if (! s.isEmpty())
     enc = s.trimmed();

  return enc;
}


bool file_is_writable (const QString &fname)
{
  QFile f (fname);
  return f.isWritable();
}


bool file_is_readable (const QString &fname)
{
  QFile f (fname);
  return f.isReadable();
}


bool dir_exists (const QString &path)
{
  QDir d (path);
  return d.exists();
}


bool file_exists (const QString &fileName)
{
  if (fileName.isEmpty())
     return false;

  return QFile::exists (fileName);
}


QString change_file_ext (const QString &s, const QString &ext)
{
  int i = s.lastIndexOf (".");
  if (i == -1)
     return (s + "." + ext);

  QString r (s);
  r.truncate (++i);

  r.append (ext);
  return r;
}


QString file_get_ext (const QString &file_name)
{
  if (file_name.isEmpty())
      return QString();

  int i = file_name.lastIndexOf (".");
  if (i == -1)
      return QString();

  return file_name.mid (i + 1).toLower();
}


QStringList read_dir_entries (const QString &path)
{
  QDir dir (path);
  return dir.entryList (QDir::AllEntries | QDir::NoDotAndDotDot);
}


/* io utils */


bool qstring_save (const QString &fileName, const QString &data, const char *enc)
{
/*  QFile file (fileName);
  if (! file.open (QFile::WriteOnly | QFile::Text))
      return false;

  QTextStream out (&file);
  out.setCodec (enc);
  out << data;
*/
  QFile file (fileName);
  if (! file.open (QFile::WriteOnly))
      return false;

  QTextCodec *codec = QTextCodec::codecForName (enc);
  QByteArray ba = codec->fromUnicode(data);

  file.write(ba);
  file.close();

  return true;
}


QString qstring_load (const QString &fileName, const char *enc)
{
 /* QFile file (fileName);

  if (! file.open (QFile::ReadOnly | QFile::Text))
     return QString();

  QTextStream in(&file);
  in.setCodec (enc);

  return in.readAll();*/

  QFile file (fileName);

//  if (! file.open (QFile::ReadOnly | QIODevice::Text))
  if (! file.open (QFile::ReadOnly))
      return QString();

//  QTextStream in (&file);
 // in.setCodec (charset.toUtf8().data());

  QByteArray ba = file.readAll();
  QTextCodec *codec = QTextCodec::codecForName(enc);

  file.close();

  return codec->toUnicode(ba);

}


QString qstring_load_first_line (const QString &fileName)
{
  QFile file (fileName);

  if (! file.open (QFile::ReadOnly | QFile::Text))
     return QString();

  QTextStream in(&file);

  return in.readLine();
}


QByteArray file_load (const QString &fileName)
{
  QFile file (fileName);
  QByteArray b;

  if (! file.open (QFile::ReadOnly))
      return b;

  b = file.readAll();

  return b;
}


/* string/stringlist utils */


void strlist_swap (QStringList &l, int a, int b)
{
  QString t = l[a];
  l[a] = l[b];
  l[b] = t;
}


QString string_between (const QString &source,
                        const QString &sep1,
                        const QString &sep2)
{
  QString result;
  int pos1 = source.indexOf (sep1);
  if (pos1 == -1)
     return result;

  int pos2 = source.indexOf (sep2, pos1 + sep1.size());
  if (pos2 == -1)
     return result;

  pos1 += sep1.size();

  result = source.mid (pos1, pos2 - pos1);
  return result;
}


bool char_is_bad (const QChar &c)
{
  if (! c.isNull() && ! c.isLetter())
     return true;

  return false;
}


void qstring_list_print (const QStringList &l)
{
  for (int i = 0; i < l.size(); i++)
      qDebug() << l[i];
}


QStringList bytearray_to_stringlist (const QList<QByteArray> &a)
{
  QStringList r;

  for (int i = 0; i < a.size(); i++)
       r.append (a.at(i).data());

  return r;
}


/* hash utils */

QString hash_get_val (QHash<QString, QString> &h,
                      const QString &key,
                      const QString &def_val)
{
  QString result = h.value (key);
  if (result.isEmpty())
     {
      result = def_val;
      h.insert (key, def_val);
     }

  return result;
}


QString qstring_load_value (const QString &fileName, const QString &key, const QString &def)
{
  QHash <QString, QString> h = hash_load_keyval (fileName);
  return hash_get_val (h, key, def);
}


QHash <QString, QString> hash_load_keyval (const QString &fname)
{
  QHash <QString, QString> result;

  if (! file_exists (fname))
     return result;

  QStringList l = qstring_load (fname).split ("\n");

  for (QList <QString>::iterator i = l.begin(); i != l.end(); ++i)
      {
       QStringList sl = i->split ("=");
       if (sl.size() > 1)
          result.insert (sl.at(0), sl.at(1));
      }

  return result;
}


/* image utils */

bool is_image (const QString &filename)
{
  if (filename.isEmpty())
     return false;

  QList <QByteArray> a = QImageReader::supportedImageFormats();

  for (QList <QByteArray>::iterator i = a.begin(); i != a.end(); ++i)
      {
       QString t (i->data());
       if (filename.endsWith (t.prepend ("."), Qt::CaseInsensitive))
          return true;
      }

  return false;
}


QString get_insert_image (const QString &file_name, const QString &full_path, const QString &markup_mode)
{
  if (! is_image (full_path))
     return QString();

  QFileInfo inf (file_name);
  QDir dir (inf.absolutePath());

  QImage img;
  img.load (full_path);
  QString result;

  if (markup_mode == "HTML")
     result = QString ("<img src=\"%1\" alt=\"\" width=\"%2\" height=\"%3\">").arg (
                        dir.relativeFilePath (full_path)).arg (img.width()).arg (img.height());

  else
  if (markup_mode == "XHTML")
     result = QString ("<img src=\"%1\" border=\"0\" alt=\"\" width=\"%2\" height=\"%3\" />").arg (
                       dir.relativeFilePath (full_path)).arg (img.width()).arg (img.height());
  else
  if (markup_mode == "Docbook")
     result = QString ("<mediaobject><imageobject>\n<imagedata fileref=\"%1\"/>\n</imageobject></mediaobject>").arg (
                        dir.relativeFilePath (full_path)) ;
  else
  if (markup_mode == "LaTeX")
      result = QString ("\\includegraphics{%1}").arg (dir.relativeFilePath (full_path));
  else
  if (markup_mode == "Lout")
      result = QString ("@IncludeGraphic {%1}").arg (dir.relativeFilePath (full_path));

  return result;
}


/* class functions */

void CFilesList::iterate (QFileInfo &fi)
{
  if (fi.isDir())
     {
      QDir d (fi.absoluteFilePath());
      QFileInfoList lfi= d.entryInfoList (QDir::Dirs | QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);
      for (int i = 0; i < lfi.count(); i++)
           iterate (lfi[i]);
     }
  else
  if (fi.isFile())
     list.append (fi.absoluteFilePath());
}


void CFilesList::get (const QString &path)
{
  list.clear();
  QDir d (path);
  QFileInfoList lfi= d.entryInfoList (QDir::Dirs | QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);
  for (int i = 0; i < lfi.count(); i++)
      iterate (lfi[i]);
}


CFTypeChecker::CFTypeChecker (const QStringList &fnames, const QStringList &exts)
{
  lexts = exts;
  lnames = fnames;
}


bool CFTypeChecker::check (const QString &fname) const
{
  bool result = false;

#if QT_VERSION >= 0x050000
  QMimeType mime = db.mimeTypeForFile(fname);
  if (mime.inherits ("text/plain"))
     result = true;
#endif

  if (! result)
      result = lnames.contains (fname.toLower());

  if (! result)
     {
      QString ext = file_get_ext (fname);
      result = lexts.contains (ext.toLower());
     }

  return result;
}


QStringList CFTypeChecker::get_supported_exts() const
{
  QStringList l;

  l += lexts;
  l.append ("txt");
  return l;
}

