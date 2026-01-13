using System;
using System.Diagnostics.CodeAnalysis;
using System.IO;

using codegen;

if (args.Length < 3)
{
    Console.WriteLine("Missing args");
    return;
}

string OutputType = args[0];

string SchemaDir = args[1];
if (string.IsNullOrEmpty(SchemaDir) || !Directory.Exists(SchemaDir))
{
    Console.WriteLine("Input Dir " + SchemaDir + " does not exit");
    return;
}

string OutputDir = args[2];
if (string.IsNullOrEmpty(OutputDir) || !Directory.Exists(OutputDir))
{
    Console.WriteLine("Output Dir " + OutputDir + " does not exit");
    return;
}



foreach (var file in Directory.GetFiles(SchemaDir, "*.schema"))
{
    List<ClassInfo> classes = new List<ClassInfo>();

    string outputFileName = Path.GetFileNameWithoutExtension(file);

    var reader = File.OpenText(file);

    string NamespaceString = string.Empty;

    ClassInfo? currentClass = null;

    var text = reader.ReadToEnd();
    text = text.Replace("\r", string.Empty);

    foreach ( var line in text.Split(new char[] { '\n' }))
    {
        if (string.IsNullOrEmpty(line))
            continue;

        var newLine = line.Trim();

        if (string.IsNullOrEmpty(newLine))
            continue;

        var parts = newLine.Split(new char[] { ' ', '\t' },2);

        if (parts.Length < 1)
            continue;

        var keyword = parts[0].ToLower();
        if (keyword == "namespace")
        {
            NamespaceString = parts[1];
        }
        else
        {
            if (currentClass  == null)
            {
                currentClass = new ClassInfo();

                currentClass.ClassType = parts[0];
                currentClass.Name = parts[1];
            }
            else
            {
                if (keyword == "}")
                {
                    classes.Add(currentClass);
                    currentClass = null;
                }
                else if (keyword != "{")
                {
                    if (keyword.StartsWith("["))
                    {
                        currentClass.Metadata.Add(keyword, parts[1]);
                    }
                    else
                    {
                        FieldInfo field = new FieldInfo();
                        field.FieldTypename = parts[0];
                        string name = parts[1].TrimEnd(';');
                        if (name.Contains("="))
                        {
                            var nameParts = name.Split(new char[] { '=' });
                            name = nameParts[0].Trim();
                            field.DefaultValue = nameParts[1].Trim();
                        }

                        field.Name = name;

                        currentClass.Fields.Add(field);
                    }
                }
            }
        }
    }


    string outPath = Path.Combine(OutputDir, outputFileName + ".h");

    switch (OutputType)
    {
        case "c++":
            CPPGenerator.Generate(classes, NamespaceString, outPath);
            break;

        default:
            Console.WriteLine("Output mode " + OutputType + " is unknown");
            break;
    }
}

