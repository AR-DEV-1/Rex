using Sharpmake;
using System.IO;

[Generate]
public class RexUnitTest : TestProject
{
  public RexUnitTest() : base()
  {
    Name = GenerateName("RexUnitTest");
    GenerateTargets();

    string ThisFileFolder = Path.GetDirectoryName(Utils.CurrentFile());
    SourceRootPath = ThisFileFolder;
  }

  protected override void SetupLibDependencies(RexConfiguration conf, RexTarget target)
  {
    base.SetupLibDependencies(conf, target);

    conf.AddPublicDependency<RexStd>(target, DependencySetting.Default);
    conf.AddPublicDependency<RexEngine>(target, DependencySetting.Default);

    // We also need to add the private includes of rex engine 
    // as we need to do a minimal engine initialization for a few things
    // (eg. the global allocators)
    string rexEngineSourceRoot = Path.Combine(Globals.Root, "source", "1_engine", "rex_engine");
    AddPrivateIncludeIfExists(conf, ($@"{rexEngineSourceRoot}\include\private"));
  }

  protected override void SetupConfigSettings(RexConfiguration conf, RexTarget target)
  {
    base.SetupConfigSettings(conf, target);

    conf.Options.Remove(Options.Vc.Compiler.JumboBuild.Enable);
  }

  protected override void SetupOutputType(RexConfiguration conf, RexTarget target)
  {
    conf.Output = Configuration.OutputType.Lib;
  }
}
