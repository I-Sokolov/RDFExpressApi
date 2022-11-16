using System;
using CommandLine;


namespace RDFWrappers
{
    class Program
    {
        public class Options
        {
            [CommandLine.Option("embedded", Default = false, HelpText = "Generate wrapper classes for all embedded schemas (IFC2x3, IFC4, IFC4x4, CIS2, AP242)")]
            public bool embedded { get; set; }

            [CommandLine.Option("schema", HelpText = "Generate wrapper for by EXPRESS file path or schema name")]
            public string schema { get; set; }

            [CommandLine.Option("cs", HelpText = "Pathnamte of c# file to be generate, or '0' to suppress c# generation. Default is schema file name.")]
            public string csFile { get; set; }

            [CommandLine.Option('h', HelpText = "Pathname of c++ header file to be generated, or '0' to suppress c++ generation. Default is schema file name.")]
            public string hFile { get; set; }
            
            [CommandLine.Option("namespace", HelpText = "Namespase. Default is schema file name.")]
            public string Namespace { get; set; }

            [CommandLine.Option("printSchema", Default = false, HelpText = "Print parsed schema.")]
            public bool printSchema { get; set; }
        }

        static int Main(string[] args)
        {
            try
            {
                Console.WriteLine("This application generates C# and C++ header files with helper classes to work with EXPRESS models.");

                Options options = null;
                Parser.Default.ParseArguments<Options>(args).WithParsed<Options>(o => { options = o; });
                if (options == null)
                {
                    return -1;//--help option
                }

                bool done = false;

                if (options.embedded)
                {
                    GenerateForEmbedded();
                    done = true;
                }

                if (!string.IsNullOrEmpty(options.schema))
                {
                    Generate(options);
                    done = true;
                }

                //
                if (!done)
                {
                    Console.WriteLine("\nNothing generated.\nSpecify command line option --embedded or --schema.\n");
                    Console.WriteLine("Use --help to ptint more information and visit http://www.rdf.bg.");
                    return -1;
                }

                return 0;
            }
            catch (Exception e)
            {
                Console.WriteLine();
                Console.WriteLine("EXCEPTION ERROR:");
                Console.WriteLine(e.ToString());
                return 13;
            }
        }

        private static void GenerateForEmbedded()
        {
            Generate("IFC2x3");
            Generate("IFC4");
            Generate("IFC4x3");
            Generate("IFC4x4");
            Generate("AP242");
            Generate("CIS2");
        }

        private static void Generate (string schemaName)
        {
            var options = new Options();
            options.schema = schemaName;

            Generate(options);
        }

        private static void Generate(Options options)
        {
            Console.WriteLine("\n\nGenerating classes for " + options.schema);

            string baseName = System.IO.Path.GetFileNameWithoutExtension(options.schema);

            string defaultOutDir = @"..\..\..\API.generated\";
            if (!System.IO.Directory.Exists(defaultOutDir))
            {
                defaultOutDir = "";
            }

            if (string.IsNullOrWhiteSpace(options.csFile))
            {
                options.csFile = defaultOutDir + baseName + ".cs";
                options.csFile = System.IO.Path.GetFullPath(options.csFile);
            }

            if (string.IsNullOrWhiteSpace(options.hFile))
            {
                options.hFile = defaultOutDir + baseName + ".h";
                options.hFile = System.IO.Path.GetFullPath(options.hFile);
            }

            if (string.IsNullOrWhiteSpace(options.Namespace))
            {
                options.Namespace = baseName;
            }

            options.Namespace = Generator.ValidateIdentifier(options.Namespace);

            //
            // Main course
            //
            var schema = new Schema(options.schema);

            Console.WriteLine("....");

            ///
            ///
            if (options.printSchema)
            {
                schema.ToConsole();
                Console.WriteLine();
            }

            ///
            ///
            if (!string.IsNullOrWhiteSpace(options.hFile) && options.hFile != "0")
            {
                Console.WriteLine("Generate C++ header file " + options.hFile);
                Generator cppgen = new Generator(schema, false, options.Namespace);
                cppgen.WriteWrapper(options.hFile);
            }
            else
            {
                Console.WriteLine("Options says do not create C++ header file");
            }
            System.Console.WriteLine();

            ///
            ///
            if (!string.IsNullOrWhiteSpace(options.hFile) && options.csFile != "0")
            {
                Console.WriteLine("Generate C# code " + options.csFile);
                Generator csgen = new Generator(schema, true, options.Namespace);
                csgen.WriteWrapper(options.csFile);
            }
            else
            {
                Console.WriteLine("Options says do not create C++ header file");
            }
            System.Console.WriteLine();
        }
    }
}
