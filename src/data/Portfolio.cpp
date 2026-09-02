#include "data/Portfolio.hpp"

namespace itsme::data {
using core::Tone;

const Profile& profile() {
  static const Profile p{
      "Leo Felcianas",
      "DevOps Engineer | Software Engineer | Freelancer",
      "DevOps Engineer & Software Engineer",
      "Colombo, Western Province, Sri Lanka",
      "leogavin123@outlook.com",
      "https://www.linkedin.com/in/leogavin/",
      "linkedin.com/in/leogavin",
      "https://github.com/DFanso",
      "github.com/DFanso",
      "https://itsme.dfanso.dev/",
      "itsme.dfanso.dev",
      "https://itsme.dfanso.dev/resume.pdf",
      "Senior Software Engineer at CD Extreme OPC, holding a First-Class Honours degree from the University of "
      "Plymouth. Experienced across DevOps pipelines, backend development, cloud infrastructure, and AI-driven "
      "automation. Co-Founder & CTO of CodeXeed and KlexD, building scalable cloud-native applications and "
      "intelligent systems for global clients.",
  };
  return p;
}

const std::vector<Company>& companies() {
  static const std::vector<Company> c = {
      {"CD Extreme OPC",
       "March 2026 - Present",
       {{"SWE",
         Tone::Blue,
         "Senior Software Engineer",
         "March 2026 - Present",
         {"Developing and maintaining high-performance applications using .NET Core and Clean Architecture "
          "principles",
          "Integrating Azure DevOps pipelines for CI/CD and leveraging Azure Web Services for cloud deployments",
          "Applying architectural patterns including Mediator, CQRS, and Domain-Driven Design (DDD)",
          "Working with Entity Framework Core and PostgreSQL for robust, scalable data access layers",
          "Delivering innovative software solutions and game-related applications for a global audience",
          "Built a KYC (Know Your Customer) verification platform for 747Live, covering document and "
          "facial-similarity verification workflows",
          "Trained and fine-tuned custom Python ML models for automated identity verification and fraud detection",
          "Designed load-balanced, auto-scaling infrastructure across Cloudflare and Azure to handle "
          "high-throughput verification traffic",
          "Set up Azure DevOps (ADO) pipelines for CI/CD of the KYC platform and automated ML model deployment"},
         {".NET Core", "C#", "Azure DevOps", "Azure", "EF Core", "PostgreSQL", "Clean Arch", "Mediator", "Python",
          "PyTorch", "Cloudflare", "Docker", "Redis"}}}},
      {"Empite",
       "July 2024 - March 2026 · 1 yr 8 mos",
       {{"DEV",
         Tone::Yellow,
         "DevOps Engineer",
         "August 2025 - March 2026",
         {"Implemented auto-scaling solutions, achieving a 20% cost reduction while maintaining high availability",
          "Executed disaster recovery plans and led migrations to immutable infrastructure using Terraform",
          "Conducted load testing using Artillery, improving infrastructure scalability by 70%",
          "Collaborated with developers to optimize codebases, ensuring systems handled increased traffic",
          "Enhanced CI/CD pipelines and containerized services, accelerating deployments"},
         {"AWS", "Azure", "Linux", "Terraform", "Packer", "Go", "K8s", "Docker", "Artillery", "Rapid7"}},
        {"DEV",
         Tone::Yellow,
         "Associate DevOps Engineer",
         "November 2024 - July 2025",
         {"Migrated legacy project to modern Docker/ECS, reducing costs by 3x and enhancing maintainability",
          "Built Azure IaC with Terraform, reducing provisioning time by 60% and standardizing deployments",
          "Recovered an abandoned project, designing the infrastructure charter and deploying to production",
          "Refactored legacy Terraform scripts to integrate with modern services for easier scaling",
          "Designed and executed artillery-based load testing and auto-scaling improvements"},
         {"AWS", "Nest.js", "Go", "Linux", "TypeScript", "Azure DevOps", "Terraform", "Artillery"}},
        {"INT",
         Tone::Green,
         "Software Engineer Intern",
         "July 2024 - October 2024",
         {"Developed NestJS microservices backends and integrated Next.js frontends",
          "Optimized code performance by refactoring core services, reducing latency",
          "Contributed to microservices-based architecture, enhancing modularity",
          "Assisted in CI/CD pipeline improvements and containerization of services"},
         {"AWS", "React", "Nest.js", "Node.js", "Linux", "Git"}}}},
      {"CodeXeed",
       "June 2025 - Present · 11 mos",
       {{"CTO",
         Tone::Red,
         "Co-Founder & CTO",
         "June 2025 - Present",
         {"Defining the tech stack and architectural patterns for client projects, from discovery to deployment",
          "Driving development of enterprise-grade web applications using Next.js, NestJS, and PostgreSQL",
          "Implementing high-performance SEO-optimized structures and robust security measures",
          "Leading legacy system revamps, migrating Angular/PHP monoliths to Next.js microservices"},
         {"Next.js", "Nest.js", "PostgreSQL", "Docker", "AWS", "Terraform"}}}},
      {"KlexD",
       "October 2024 - Present · 1 yr 7 mos",
       {{"CTO",
         Tone::Red,
         "Co-Founder & CTO",
         "October 2024 - Present",
         {"Architecting intelligent systems and smart assistants using Python, OpenAI, and LangChain",
          "Steering development of mobile and web platforms using React Native and Next.js",
          "Building robust REST/GraphQL API pipelines and ETL processes to connect business tools",
          "Establishing high-quality code culture through QA testing, Docker, and CI/CD pipelines"},
         {"Python", "OpenAI", "LangChain", "React Native", "Next.js", "Docker"}}}},
      {"Fiverr",
       "January 2020 - May 2025 · 5 yrs 5 mos",
       {{"FRL",
         Tone::Purple,
         "Software Engineer",
         "May 2023 - May 2025",
         {"Delivering custom web and mobile solutions for diverse client requirements",
          "Implementing secure, scalable full-stack architectures with Go and Node.js",
          "Managing end-to-end project lifecycle from requirements to deployment"},
         {"AWS", "Next.js", "Nest.js", "Go", "Linux", "Prisma", "Tailwind", "Firebase"}},
        {"VID",
         Tone::Orange,
         "Video Editor",
         "January 2020 - May 2021",
         {"Produced and edited professional video content for international clients",
          "Managed end-to-end video production workflow from raw footage to final delivery"},
         {"Premiere Pro", "After Effects"}}}},
      {"Melstasoft",
       "June 2022 - August 2022 · 3 mos",
       {{"INT",
         Tone::Green,
         "Software Engineer Intern",
         "June 2022 - August 2022",
         {"Developed and maintained enterprise applications using ASP.NET and the .NET framework",
          "Worked with MSSQL databases, writing queries and managing data models",
          "Collaborated with senior engineers to deliver production-ready features"},
         {".NET", "ASP.NET", "MSSQL", "C#", "Git"}}}},
      {"FOSS Community - NSBM",
       "June 2020 - March 2023 · 2 yrs 10 mos",
       {{"VOL",
         Tone::Teal,
         "Digital Marketing Team Lead",
         "February 2022 - March 2023",
         {"Led the digital marketing team, managing campaigns and community outreach",
          "Coordinated with council members to align marketing with community events"},
         {"Social Media", "Design"}},
        {"VOL",
         Tone::Teal,
         "Council Member",
         "June 2021 - March 2023",
         {"Served as an active council member, driving open-source initiatives and community growth",
          "Organized and participated in FOSS events, workshops, and hackathons"},
         {"Open Source", "Linux"}},
        {"VOL",
         Tone::Teal,
         "Volunteer",
         "June 2020 - June 2021",
         {"Volunteered in community activities, supporting FOSS events and initiatives"},
         {"Open Source"}}}},
  };
  return c;
}

const std::vector<EducationEntry>& education() {
  static const std::vector<EducationEntry> e = {
      {"BSc in Computer Software Engineering", "University of Plymouth", "First-Class Honours",
       "June 2021 - December 2024", "Plymouth, United Kingdom"},
      {"Foundation Program for Bachelor's Degree", "NSBM Green University", "", "March 2020 - April 2021",
       "Sri Lanka"},
  };
  return e;
}

const std::vector<Certification>& certifications() {
  static const std::vector<Certification> c = {
      {"Multicloud Network Associate", "Aviatrix", "Issued Sep 2025", "Expires Sep 2028",
       "Credential ID 2025-27675"},
      {"AWS Cloud Practitioner Essentials", "Amazon Web Services (AWS)", "Issued May 2025", "", ""},
  };
  return c;
}

const std::vector<SkillCategory>& skillCategories() {
  static const std::vector<SkillCategory> s = {
      {"cloud", {"AWS", "GCP", "Azure", "Cloudflare"}},
      {"containers", {"Docker", "Kubernetes", "K3s", "ECS", "ELK"}},
      {"infra",
       {"Linux", "Helm", "Nginx", "Terraform", "Packer", "Ansible", "Jenkins", "GitHub Actions", "ArgoCD", "GitOps",
        "Artillery"}},
      {"lang", {"Go", "Rust", "Python", "TypeScript", "JavaScript", "Node.js", "C#"}},
      {"frameworks",
       {"Next.js", "FastAPI", "11ty", "Nest.js", "Fiber", "HTMX", "Tailwind", "GraphQL", "React Native", ".NET Core",
        "EF Core"}},
      {"ai", {"OpenAI", "LangChain", "AI Automation"}},
      {"db", {"PostgreSQL", "MySQL", "MongoDB", "Redis", "Firebase", "Prisma", "GORM"}},
      {"serverless", {"Vercel", "App Runner", "App Service", "Lambda", "Workers", "Convex"}},
      {"monitor", {"Prometheus", "Grafana", "Loki", "Tempo", "Mimir", "ELK Stack", "Rapid7"}},
      {"arch",
       {"Microservices", "System Design", "Network Security", "Technical Product Management", "REST / GraphQL APIs",
        "Clean Architecture", "CQRS / Mediator"}},
  };
  return s;
}

const std::vector<Project>& projects() {
  static const std::vector<Project> p = {
      {"OPS", "eks-gitops-platform",
       "https://github.com/DFanso?tab=repositories&q=DevOps-Project-001&type=&language=&sort=",
       std::string("DFanso/DevOps-Project-001"),
       "Production-grade 3-repo GitOps platform on AWS EKS with Terraform IaC, GitHub Actions CI, and ArgoCD for "
       "continuous deployment with DataDog observability.",
       {"AWS EKS", "Terraform", "Kubernetes", "Helm", "ArgoCD", "DataDog", "GitHub Actions", "Go"}},
      {"OPS", "k3s-cicd", "https://github.com/DFanso/k3s", std::string("DFanso/k3s"),
       "A complete Kubernetes deployment setup with automated CI/CD using GitHub Actions, Helm, and full "
       "observability stack.",
       {"K3s", "GitHub Actions", "Helm", "Prometheus", "Grafana", "Nginx", "FastAPI"}},
      {"WEB", "techxeed", "https://www.techxeed.com/", std::nullopt,
       "A comprehensive digital solutions platform offering web development, mobile apps, AI solutions, and digital "
       "marketing services.",
       {"Next.js", "Nest.js", "MongoDB", "TailwindCSS", "Firebase", "AWS", "Stripe", "Terraform"}},
      {"APP", "quickquest", "https://github.com/DFanso/QuickQuest", std::string("DFanso/QuickQuest"),
       "A location-based platform connecting customers with laborers, featuring real-time chat via SSE and "
       "geospatial queries.",
       {"Next.js", "Nest.js", "MongoDB", "Python", "AWS", "PayPal"}},
      {"APP", "rss-reader", "https://github.com/DFanso/rss", std::string("DFanso/rss"),
       "A simple, modern RSS reader and generator built with Go and HTMX, featuring persistent storage and a clean "
       "UI.",
       {"Go", "HTMX", "TailwindCSS"}},
      {"CLI", "commit-msg", "https://github.com/DFanso/commit-msg", std::string("DFanso/commit-msg"),
       "AI-powered CLI tool that generates conventional commit messages using various LLMs including Gemini, Grok, "
       "and OpenAI.",
       {"Go", "LLMs"}},
      {"OPS", "aws-ecs-infra", "https://github.com/DFanso/aws-ecs-infrastructure",
       std::string("DFanso/aws-ecs-infrastructure"),
       "Infrastructure as Code configurations for deploying scalable applications on AWS ECS using Terraform.",
       {"Terraform", "AWS"}},
  };
  return p;
}

const std::vector<ContactLink>& contactLinks() {
  static const std::vector<ContactLink> c = {
      {"EM", "leogavin123@outlook.com", std::string("mailto:leogavin123@outlook.com")},
      {"LO", "Colombo, Sri Lanka", std::nullopt},
      {"GH", "github.com/DFanso", std::string("https://github.com/DFanso")},
      {"IN", "linkedin.com/in/leogavin", std::string("https://www.linkedin.com/in/leogavin/")},
      {"DC", "discord", std::string("https://discord.gg/DcFFdcjfAf")},
      {"IG", "instagram.com/dfansoo", std::string("https://instagram.com/dfansoo")},
  };
  return c;
}

}  // namespace itsme::data
